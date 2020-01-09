#include "../include/acquirepublisher.h"
#include "../include/definitions.h"
#include <ctime>
#include <chrono>
#include <iostream>
using std::cout;
using std::cerr;

void AcquirePublisher::start(UimfRequestMessage uimf)
{
	if (should_stop)
		return;

	worker_handle = std::thread([&, uimf]()
	{
		int total_triggers = uimf.frame_length() * uimf.nbr_accumulations();
		int triggers_acquired = 0;

		//std::cout << "Starting frame acquisition" << std::endl;

		state = State::ACQUIRING;

#if TIMING_INFORMATION
		auto start = std::chrono::high_resolution_clock::now();
		std::cout << "START ACQUIRE FRAME: " << timestamp_now() << std::endl;
#endif
		context->start();
		while (triggers_acquired < total_triggers && !should_stop)
		{
			try
			{
				auto data = context->acquire(std::chrono::milliseconds(1000));
				triggers_acquired += data.stamps.size();
				notify(data, SubscriberType::BOTH);
			}
			catch (std::string ex)
			{
				std::cerr << "data acquire failure -- " << ex << std::endl;
				stop_signal.set_value(State::ERRORED);
				goto stop;
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

void AcquirePublisher::start()
{
	if (should_stop)
		return;

	worker_handle = std::thread([&]()
	{
		state = State::ACQUIRING;

		context->start();
		while (!should_stop)
		{
			try
			{
				auto data = context->acquire(std::chrono::milliseconds(250));
				
				if (context->get_available_buffers() > 19)
				{
					notify(data, SubscriberType::ACQUIRE);
				}
				else
					std::cerr << "dropping " <<  data.stamps.size() << " scans\n";

			}
			catch (std::string ex)
			{
				std::cerr << "data acquire failure -- " << ex << std::endl;
				stop_signal.set_value(State::ERRORED);
				goto stop;
			}
		}

		stop_signal.set_value(State::STOPPED);
stop:
		context->stop();
	});
}

void AcquirePublisher::stop()
{
	try
	{
		if (should_stop)
			return;

		//std::cout << "Stopping acquisition" << std::endl;
		auto fut = stop_signal.get_future();
		should_stop = true;
			
		fut.wait();			
		state = fut.get();
		if (state != State::STOPPED)
			std::cerr << "state != State::STOPPED\n";

		notify_completed_and_wait();
		worker_handle.join();

		//std::cout << "stop completed" << std::endl;
	}
	catch (...)
	{
		std::cerr << "problem stopping publisher" << std::endl;
	}
}

