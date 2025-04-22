#ifndef FRAME_SUBSCRIBER_H
#define FRAME_SUBSCRIBER_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <future>
#include <spdlog/spdlog.h>
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
class Subscriber {
private:
	std::condition_variable sig;
	std::mutex sig_sync;
	std::unique_ptr<std::thread> worker_handle;
	std::promise<void> has_completed;
	std::shared_future<void> stop_fut;
	bool is_running; // TODO: make this atomic
	std::deque<T> protected_queue;
	std::mutex queue_sync;

protected:
	std::deque<T> items;

public:
	Subscriber()
		: items()
		, worker_handle()
		, is_running(false)
	{}

	virtual ~Subscriber()
	{
		if (worker_handle && worker_handle->joinable()) {
			worker_handle->join();
		}
	};

	std::shared_future<void> setup(std::shared_future<void> pub_stop)
	{
		// For now just set the last publishers shared_future, adding a race condition
		this->stop_fut = pub_stop;

		if (!is_running)
		{
			worker_handle = std::make_unique<std::thread>([&]()
			{
				is_running = true;
				while (stop_fut.wait_for(std::chrono::seconds(0)) != std::future_status::ready || !items.empty() || !protected_queue.empty())
				{
					{
						std::unique_lock<std::mutex> lock(sig_sync);
						sig.wait_for(lock, std::chrono::milliseconds(10));
					}

					{
						const std::lock_guard<std::mutex> lock(queue_sync);
						for (int i = 0; i < protected_queue.size(); i++)
						{
							items.push_back(protected_queue.front());
							protected_queue.pop_front();
						}
					}

					while (!items.empty())
					{
						T item = items.front();
						items.pop_front();
						on_notify(item);
					}
				}

				on_completed();
				has_completed.set_value();
			});
		}
		return std::shared_future<void>(has_completed.get_future());
	}

	inline void update(T item)
	{
		const std::lock_guard<std::mutex> lock(queue_sync);
		protected_queue.push_back(item);
		sig.notify_one();
	}

private:
	virtual inline void on_notify(T& item) {};
	virtual inline void on_completed() {};
};

#endif // !FRAME_SUBSCRIBER_H
