#include "../include/acquirepublisher.h"

void AcquirePublisher::start()
{
	worker_handle = std::thread([&]()
	{
		std::cout << "AcquirePublisher :: starting acquisition" << std::endl;
		context->start();
		while (!should_stop)
		{
			if (has_errored)
			{
				//context->stop();
				//context->start();
				//has_errored = false;
				break;
			}

			try
			{
				//auto t1 = std::chrono::high_resolution_clock::now();
				auto data = context->acquire(std::chrono::milliseconds(80));
				//auto t2 = std::chrono::high_resolution_clock::now();
				//auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
				//std::cout << "\t\tTIME TO ACQUIRE: " << dur.count() << "\n";
				
				//auto t1_n = std::chrono::high_resolution_clock::now();
				notify(data, SubscriberType::BOTH);
				//auto t2_n = std::chrono::high_resolution_clock::now();
				//auto dur_n = std::chrono::duration_cast<std::chrono::milliseconds>(t2_n - t1_n);
				//std::cout << "\t\tTIME TO NOTIFY ACQUIRE: " << dur_n.count() << "\n";

			}
			catch (std::string ex)
			{
				std::cout << "data acquire failure -- " << ex << std::endl;
				has_errored = true;
			}
		}

		stop_sig.notify_one();
	});
}

void AcquirePublisher::stop()
{
	try
	{
		if (!should_stop)
		{
			should_stop = true;

			{
				std::unique_lock<std::mutex> lock(stop_mut);
				stop_sig.wait(lock);
			}

			std::cout << "AcquirePublisher :: stopping acquisition" << std::endl;
			if(!context->is_stopped())
				context->stop();
			notify_completed_and_wait();
			worker_handle.join();
		}
		std::cout << "AcquirePublisher :: stop completed" << std::endl;
	}
	catch (...)
	{
		std::cout << "problem stopping application" << std::endl;
	}
}

