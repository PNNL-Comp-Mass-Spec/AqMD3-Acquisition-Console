#ifndef SIMPLE_PUBLISHER_H
#define SIMPLE_PUBLISHER_H

#include "../framepublisher.h"
#include "../acquireddata.h"
#include "../streamingcontext.h"

#include <chrono>

class SimplePublisher : public FramePublisher<AcquiredData> {
private:
	std::unique_ptr<StreamingContext> context;

public:
	SimplePublisher(std::unique_ptr<StreamingContext> context)
		: context(std::move(context))
	{}

	void acquire_count(int count);
};

void SimplePublisher::acquire_count(int count)
{
	context->start();
	for (int i = 0; i < count; i++)
	{
		auto data = context->acquire();
		notify(data, SubscriberType::BOTH);
	}
	context->stop();
}

#endif // !SIMPLE_PUBLISHER_H