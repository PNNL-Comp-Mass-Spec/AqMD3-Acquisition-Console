#ifndef FRAME_PUBLISHER_H
#define FRAME_PUBLISHER_H

#include "framesubscriber.h"
#include <list>
#include <tuple>
#include <future>

template <typename T>
class FramePublisher {
protected:
	std::list<std::tuple<SubscriberType, std::shared_ptr<FrameSubscriber<T>>, std::shared_future<void>>> subscribers;
	std::promise<void> completed_signal;
	std::shared_future<void> completed_future;

public:
	FramePublisher()
		: completed_future(completed_signal.get_future())
	{}

	virtual ~FramePublisher() = default;

	inline void register_subscriber(std::shared_ptr<FrameSubscriber<T>> subscriber, SubscriberType type)
	{
		auto sub_finished = subscriber->setup(completed_future);
		subscribers.emplace_back(type, subscriber, sub_finished);
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
		if (completed_future.valid())
			completed_signal.set_value();
	}

	inline void notify_completed_and_wait()
	{
		notify_completed();
		
		for (const auto& subscriber : subscribers)
		{
			auto fut = std::get<2>(subscriber);
			if (fut.valid())
				fut.wait();
		}
	}

};

#endif // !FRAME_PUBLISHER_H
