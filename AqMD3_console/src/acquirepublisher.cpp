#include "../include/acquirepublisher.h"

void AcquirePublisher::start()
{
	worker_handle = std::thread([&]()
	{
		std::cout << "AcquirePublisher :: starting acquisition" << std::endl;
		context->start();
		while (!should_stop)
		{
			try
			{
				auto data = context->acquire(std::chrono::milliseconds(80));
				notify(data, SubscriberType::BOTH);
			}
			catch (std::string ex)
			{
				std::cout << "data acquire failure -- " << ex << std::endl;
				context->stop();
				break;
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

