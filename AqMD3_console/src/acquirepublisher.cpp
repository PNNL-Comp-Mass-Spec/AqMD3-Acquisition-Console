#include "../include/acquirepublisher.h"

void AcquirePublisher::start()
{
	should_stop = false;
	worker_handle = std::thread([&]()
	{
		std::cout << "starting" << std::endl;
		context->start();
		while (!should_stop)
		{
			auto data = context->acquire(std::chrono::milliseconds(80));
			notify(data, SubscriberType::BOTH);
		}
		std::cout << "stopping" << std::endl;
		context->stop();
	});
}

void AcquirePublisher::stop()
{
	std::cout << "stop requested" << std::endl;
	should_stop = true;
	worker_handle.join();
}
