#include "../include/acquireframepublisher.h"

void AcquireFramePublisher::start()
{
	if (should_stop)
		return;

	worker_handle = std::thread([&]()
	{
		int triggers_acquired = 0;

		std::cout << "AcquireFramePublisher :: starting acquisition" << std::endl;
		context->start();
		
		while (triggers_acquired < total_triggers && !should_stop)
		{
			auto data = context->acquire(std::chrono::milliseconds(80));
			triggers_acquired += data.stamps.size();

			notify(data, SubscriberType::BOTH);
		}

		std::cout << "AcquireFramePublisher :: stopping acquisition" << std::endl;
		context->stop();
	});
}

void AcquireFramePublisher::stop()
{
	if (should_stop)
		return;

	std::cout << "AcquireFramePublisher :: stop requested" << std::endl;
	should_stop = true;
	notify_completed_and_wait();
	worker_handle.join();
	std::cout << "AcquireFramePublisher :: stop completed" << std::endl;
}
