#ifndef FRAME_PUBLISHER_H
#define FRAME_PUBLISHER_H

#include "subscriber.h"
#include <list>
#include <tuple>
#include <future>

template <typename T>
class Publisher {
protected:
	std::list<std::tuple<SubscriberType, std::shared_ptr<Subscriber<T>>, std::shared_future<void>>> subscribers;
	std::promise<void> completed_signal;
	std::shared_future<void> completed_future;

public:
	Publisher()
		: completed_future(completed_signal.get_future())
	{}

	virtual ~Publisher() = default;

	inline void register_subscriber(std::shared_ptr<Subscriber<T>> subscriber, SubscriberType type)
	{
		auto sub_finished = subscriber->setup(completed_future);
		subscribers.emplace_back(type, subscriber, sub_finished);
	}

	inline void unregister_subscriber(const std::shared_ptr<Subscriber<T>>& subscriber)
	{
		auto it = std::find_if(subscribers.begin(), subscribers.end(), [&subscriber](const auto& item) {
			return std::get<1>(item) == subscriber;
		});

		if (it != subscribers.end())
		{
			subscribers.erase(it);
		}
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
		{
			completed_signal.set_value();
		}
	}

	inline void notify_completed_and_wait()
	{
		notify_completed();
		
		for (const auto& subscriber : subscribers)
		{
			auto fut = std::get<2>(subscriber);
			if (fut.valid())
			{
				fut.wait();
			}
		}
	}

};

#endif // !FRAME_PUBLISHER_H
