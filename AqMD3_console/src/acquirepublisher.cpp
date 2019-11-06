#include "../include/acquirepublisher.h"

void AcquirePublisher::start()
{
	worker_handle = std::thread([&]()
	{
		std::cout << "AcquirePublisher :: starting acquisition" << std::endl;
		context->start();
		while (!should_stop)
		{
			auto data = context->acquire(std::chrono::milliseconds(80));
			notify(data, SubscriberType::BOTH);
		}
	});
}

void AcquirePublisher::stop()
{
	std::cout << "AcquirePublisher :: stop requested" << std::endl;
	if (!should_stop)
	{
		std::cout << "AcquirePublisher :: stopping acquisition" << std::endl;
		context->stop();
		should_stop = true;
		notify_completed_and_wait();
		worker_handle.join();
	}
	std::cout << "AcquirePublisher :: stop completed" << std::endl;
}

