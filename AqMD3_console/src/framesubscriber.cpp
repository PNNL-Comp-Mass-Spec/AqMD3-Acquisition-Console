#include "..\include\framesubscriber.h"

void FrameSubscriber::on_next(std::shared_ptr<UimfFrame> frame)
{
	frames.push_back(frame);
	sig.notify_one();
}

int FrameSubscriber::do_work(bool *should_exit)
{
	while (!(*should_exit))
	{
		{
			std::unique_lock<std::mutex> lock(mut);
			sig.wait(lock);
		}

		execute();
	}

	return 0;
}