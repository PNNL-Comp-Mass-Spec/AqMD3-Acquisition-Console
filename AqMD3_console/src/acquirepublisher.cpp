#include "../include/acquirepublisher.h"
#include "../include/definitions.h"
#include <ctime>
#include <chrono>
#include <iostream>
#include <spdlog/spdlog.h>
#include <format>

#define FREE_ON_BUFFER_COUNT 5

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

				state = State::ACQUIRING;
				spdlog::info(std::format("Frame {} - Total Scan Count: {}", uimf.frame_number(), total_triggers));

#if TIMING_INFORMATION
				auto start = std::chrono::high_resolution_clock::now();
				std::cout << "START ACQUIRE FRAME: " << timestamp_now() << std::endl;
#endif
				context->start();
				while (triggers_acquired < total_triggers && !should_stop)
				{
					try
					{
						auto data = context->acquire(std::chrono::milliseconds::zero());
						triggers_acquired += data.stamps.size();
						notify(data, SubscriberType::BOTH);
					}
					catch (const std::exception& ex)
					{
						spdlog::error("Error when acquiring UIMF data: " + std::string(ex.what()));
					}
					catch (...)
					{
						spdlog::error("Unknown error when acquiring UIMF data");
					}
				}

				stop_signal.set_value(State::STOPPED);
			stop:
				context->stop();

#if TIMING_INFORMATION
				auto stop = std::chrono::high_resolution_clock::now();
				auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
				auto file_name = uimf.file_name();
				auto frame_num = uint32_t(uimf.frame_number());
				auto countMs = ms.count();
				std::cout << "END ACQUIRE FRAME: " << timestamp_now() << " -- DURATION (ms): " << countMs << std::endl;
#endif

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
				context->start();
				while (!should_stop)
				{
					auto data = context->acquire(std::chrono::milliseconds::zero());
					auto available = context->get_available_buffers();

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

				context->stop();
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
		if (should_stop)
			return;

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
	catch (const std::exception& ex)
	{
		spdlog::error("Error when stopping publisher: " + std::string(ex.what()));
	}
	catch (...)
	{
		spdlog::error("Uknown error when stopping publisher");
	}
}

