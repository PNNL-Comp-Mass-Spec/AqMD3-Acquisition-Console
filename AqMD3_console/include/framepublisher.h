#ifndef FRAME_PUBLISHER_H
#define FRAME_PUBLISHER_H

#include "framesubscriber.h"
#include "streamingcontext.h"
#include <UIMFWriter/uimfframe.h>
#include <list>
#include <tuple>

template <typename T>
class FramePublisher {
protected:
	std::list<std::tuple<SubscriberType, std::shared_ptr<FrameSubscriber<T>>>> subscribers;
	std::condition_variable sig;
	std::mutex mut;

	bool should_stop;

public:
	FramePublisher()
		: should_stop(false)
	{}

	inline void register_subscriber(std::shared_ptr<FrameSubscriber<T>> subscriber, SubscriberType type)
	{
		subscribers.emplace_back(type, subscriber);
		subscriber->setup(&should_stop);
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

};

#endif // !FRAME_PUBLISHER_H
