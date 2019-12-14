#ifndef FRAME_SUBSCRIBER_H
#define FRAME_SUBSCRIBER_H

#include <UIMFWriter/uimfframe.h>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <future>

#include <iostream>
using std::cerr;

enum class SubscriberType : uint8_t
{
	ACQUIRE = (1 << 0),
	ACQUIRE_FRAME = (1 << 1),
	BOTH = (1 << 0)|(1 << 1),
};

constexpr enum SubscriberType operator|(const enum SubscriberType left, const enum SubscriberType right)
{
	return (enum SubscriberType)(uint8_t(left) | uint8_t(right));
}

constexpr enum SubscriberType operator&(const enum SubscriberType left, const enum SubscriberType right)
{
	return (enum SubscriberType)(uint8_t(left) & uint8_t(right));
}

template <typename T>
class FrameSubscriber {
protected:
	std::deque<T> items;
	std::condition_variable sig;
	std::mutex mut;

	std::future<void> worker_handle;

	std::promise<void> has_completed;

	bool reusable;

	std::promise<void> reusable_notifier;
	std::shared_future<void> is_reusable_stop;
	bool is_running;

public:
	FrameSubscriber(bool reusable = false)
		: items()
		, worker_handle()
		, reusable(reusable)
		, is_reusable_stop(reusable_notifier.get_future())
		, is_running(false)
	{}

	virtual ~FrameSubscriber()
	{
		if (reusable)
			reusable_notifier.set_value();
	};

	std::shared_future<void> setup(std::shared_future<void> pub_stop)
	{
		if (!is_running)
		{
			auto stop = reusable ? is_reusable_stop : pub_stop;

			worker_handle = std::async(std::launch::async, [&, stop]()
			{
				is_running = true;
				while (stop.wait_for(std::chrono::seconds(0)) != std::future_status::ready || !items.empty())
				{
					{
						std::unique_lock<std::mutex> lock(mut);
						sig.wait_for(lock, std::chrono::milliseconds(10));
					}

					try
					{
						on_notify();
					}
					catch (std::string ex)
					{
						std::cerr << ex << std::endl;
					}
				}

				on_completed();
				has_completed.set_value();
			});

			if(!reusable)
				return std::shared_future<void>(has_completed.get_future());
		}

		auto promise = std::promise<void>();
		promise.set_value();
		return std::shared_future<void>(promise.get_future());
	}

	inline void update(T item)
	{
		items.push_back(item);
		sig.notify_one();
	}

private:
	virtual inline void on_notify() = 0;
	virtual inline void on_completed() = 0;
	// virtual inline void on_error() = 0;
};

#endif // !FRAME_SUBSCRIBER_H
