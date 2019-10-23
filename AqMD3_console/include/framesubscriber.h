#ifndef FRAME_SUBSCRIBER_H
#define FRAME_SUBSCRIBER_H

#include <UIMFWriter/uimfframe.h>
#include <deque>
#include <mutex>
#include <condition_variable>

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

	std::thread worker_thread;

public:
	FrameSubscriber()
		: items()
		, worker_thread()
	{}

	void setup(const bool *should_stop)
	{
		worker_thread = std::thread([&, should_stop]()
		{
			while (!(*should_stop))
			{
				{
					std::unique_lock<std::mutex> lock(mut);
					sig.wait_for(lock, std::chrono::milliseconds(10));
				}

				execute();
			}

			std::cout << "exiting worker thread" << std::endl;
		});

		worker_thread.detach();
	}

	inline void update(T item)
	{
		items.push_back(item);
		sig.notify_all();
	}

private:
	virtual void execute() = 0;

};

#endif // !FRAME_SUBSCRIBER_H
