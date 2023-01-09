#include "../include/acquirepublisher.h"
#include "../include/definitions.h"
#include <ctime>
#include <chrono>
#include <iostream>
#include <spdlog/spdlog.h>
#include <format>

// TODO: move to config
#define FREE_ON_BUFFER_COUNT 5
#define WARN_ON_BUFFER_COUNT 3

void AcquirePublisher::start(UimfRequestMessage uimf)
{
	try
	{
		if (should_stop)
			return;

		worker_handle = std::thread([&, uimf]()
			{
				int total_triggers = uimf.frame_length();
				int triggers_acquired = 0;
				bool has_errored = false;

				state = State::ACQUIRING;
				spdlog::info(std::format("Frame {} - Total scan count to acquire: {}", uimf.frame_number(), total_triggers));

				digitizer->start();
				while (triggers_acquired < total_triggers)
				{
					if (has_errored || should_stop.load())
					{
						break;
					}

					try
					{
						auto available = buffer_pool->get_available_buffers();
						if (available <= WARN_ON_BUFFER_COUNT)
						{
							spdlog::warn("Available buffer count {}", available);
						}

						auto data = digitizer->acquire(std::chrono::milliseconds(this->timeout));
						triggers_acquired += data.stamps.size();
						notify(data, SubscriberType::BOTH);
					}
					catch (const std::exception& ex)
					{
						spdlog::error("Error when acquiring UIMF data: " + std::string(ex.what()));
						has_errored = true;
					}
					catch (...)
					{
						spdlog::error("Unknown error when acquiring UIMF data");
						has_errored = true;
					}
				}

				stop_signal.set_value(State::STOPPED);
				digitizer->stop();
				spdlog::info(std::format("Scans acquired: {}", triggers_acquired));

			});
	}
	catch (const std::exception& ex)
	{
		spdlog::error("Error processing UIMF request: " + std::string(ex.what()));
	}
	catch (...)
	{
		spdlog::error("Unknown error when processing UIMF request");
	}
}

void AcquirePublisher::start()
{
	if (should_stop)
		return;

	worker_handle = std::thread([&]()
		{
			state = State::ACQUIRING;

			try
			{
				digitizer->start();
				while (!should_stop)
				{
					auto data = digitizer->acquire(std::chrono::milliseconds::zero());
					auto available = buffer_pool->get_available_buffers();

					// App may become unrecoverable if we run out of memory, this guard attempts to stop that from happening
					// TODO: revist this number and check
					if (available > FREE_ON_BUFFER_COUNT)
					{
						try
						{
							notify(data, SubscriberType::ACQUIRE);
						}
						catch (const std::exception& ex)
						{
							spdlog::error("Error during data acquisition: " + std::string(ex.what()));
						}
						catch (...)
						{
							spdlog::error("Uknown error during data acquisition");
						}
					}
					else
					{
						spdlog::warn("bufs: " + std::to_string(available) + " -- dropping " + std::to_string(data.stamps.size()) + " scans");
					}
				}

				digitizer->stop();
				stop_signal.set_value(State::STOPPED);
				return;
			}
			catch (const std::exception& ex)
			{
				spdlog::error("Error when attempting to control digitizer: " + std::string(ex.what()));
			}
			catch (...)
			{
				spdlog::error("Unknown error occured when attampting to control the digitizer");
			}
			stop_signal.set_value(State::ERRORED);
	});
}

void AcquirePublisher::stop()
{
	try
	{
		spdlog::info("Stopping");
		auto start = std::chrono::high_resolution_clock::now();

		if (!should_stop)
		{
			auto fut = stop_signal.get_future();
			should_stop = true;

			fut.wait();
			state = fut.get();
			if (state != State::STOPPED)
			{
				spdlog::warn("state != State::STOPPED");
			}

			notify_completed_and_wait();
			worker_handle.join();
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		spdlog::info(std::format("Time to stop: {} ms", ms));
	}
	catch (const std::exception& ex)
	{
		spdlog::error("Error when stopping publisher: " + std::string(ex.what()));
	}
	catch (...)
	{
		spdlog::error("Uknown error when stopping publisher");
	}
}

