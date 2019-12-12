#include "../include/acquirepublisher.h"

void AcquirePublisher::start(std::shared_ptr<UimfFrame> frame)
{
	if (should_stop)
		return;

	worker_handle = std::thread([&]()
	{
		int total_triggers = frame->frame_length * frame->nbr_accumulations;
		int triggers_acquired = 0;

		std::cout << "AcquireFramePublisher :: starting acquisition" << std::endl;
		state = State::ACQUIRING;
		
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
	});
}

void AcquirePublisher::start()
{
	if (should_stop)
		return;

	worker_handle = std::thread([&]()
	{
		std::cout << "AcquirePublisher :: starting acquisition" << std::endl;
		state = State::ACQUIRING;

		context->start();
		while (!should_stop)
		{
			try
			{
				auto start_0 = std::chrono::high_resolution_clock::now();
				auto data = context->acquire(std::chrono::milliseconds(250));
				auto stop_0 = std::chrono::high_resolution_clock::now();
				auto ms_0 = std::chrono::duration_cast<std::chrono::milliseconds>(stop_0 - start_0);
				std::cout << "acquire time: " << ms_0.count() << std::endl;
				std::cout << "available buffers: " << (context->get_available_buffers()) << std::endl;
				
				if (context->get_available_buffers() > 25)
				{
					auto start = std::chrono::high_resolution_clock::now();
					notify(data, SubscriberType::ACQUIRE);
					auto stop = std::chrono::high_resolution_clock::now();
					auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
					std::cout << "notify time: " << ms.count() << std::endl;
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

		std::cout << "Stopping acquisition" << std::endl;
		auto fut = stop_signal.get_future();
		should_stop = true;
			
		fut.wait();			
		state = fut.get();

		notify_completed_and_wait();
		worker_handle.join();

		std::cout << "stop completed" << std::endl;
	}
	catch (...)
	{
		std::cerr << "problem stopping application" << std::endl;
	}
}

