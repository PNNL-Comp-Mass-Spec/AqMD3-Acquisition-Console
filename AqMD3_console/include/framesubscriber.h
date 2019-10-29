#ifndef FRAME_SUBSCRIBER_H
#define FRAME_SUBSCRIBER_H

#include <UIMFWriter/uimfframe.h>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <future>

#include <iostream>

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

	std::thread worker_handle;

public:
	FrameSubscriber()
		: items()
		, worker_handle()
	{}

	virtual ~FrameSubscriber()
	{
		worker_handle.join();
	};

	void setup(std::shared_future<void> stop)
	{
		worker_handle = std::thread([&, stop]()
		{
			while (stop.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
			{
				{
					std::unique_lock<std::mutex> lock(mut);
					sig.wait_for(lock, std::chrono::milliseconds(10));
				}

				execute();
			}
		});
	}

	inline void update(T item)
	{
		items.push_back(item);
		sig.notify_one();
	}

private:
	virtual void execute() = 0;
};

#endif // !FRAME_SUBSCRIBER_H
