#ifndef FRAME_PUBLISHER_H
#define FRAME_PUBLISHER_H

#include "framesubscriber.h"
#include "streamingcontext.h"
#include <UIMFWriter/uimfframe.h>
#include <list>
#include <tuple>
#include <future>

template <typename T>
class FramePublisher {
protected:
	std::list<std::tuple<SubscriberType, std::shared_ptr<FrameSubscriber<T>>>> subscribers;
	std::promise<void> completed_signal;
	std::shared_future<void> completed_future;

public:
	FramePublisher()
		: completed_future(completed_signal.get_future())
	{}

	virtual ~FramePublisher() = default;

	inline void register_subscriber(std::shared_ptr<FrameSubscriber<T>> subscriber, SubscriberType type)
	{
		subscribers.emplace_back(type, subscriber);
		subscriber->setup(completed_future);
	}

protected:
	inline void notify(T item, SubscriberType type)
	{
		for (auto& sub : subscribers)
		{
			if (uint8_t(type & std::get<0>(sub)))
			{
				std::get<1>(sub)->update(item);
			}
		}
	}

	inline void notify_completed() 
	{
		completed_signal.set_value();
	}

};

#endif // !FRAME_PUBLISHER_H
