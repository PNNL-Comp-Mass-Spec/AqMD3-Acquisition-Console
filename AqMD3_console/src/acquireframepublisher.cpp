#include "../include/acquireframepublisher.h"

void AcquireFramePublisher::start()
{
	worker_handle = std::thread([&]()
	{
		int triggers_acquired = 0;

		std::cout << "AcquireFramePublisher :: starting acquisition" << std::endl;
		context->start();
		
		while (triggers_acquired < frame->frame_length)
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
	std::cout << "AcquireFramePublisher :: stop requested" << std::endl;
	notify_completed();
	worker_handle.join();
	std::cout << "AcquireFramePublisher :: stop completed" << std::endl;
}
