#ifndef FRAME_SUBSCRIBER_H
#define FRAME_SUBSCRIBER_H

#include <UIMFWriter/uimfframe.h>
#include <deque>
#include <mutex>
#include <condition_variable>

class FrameSubscriber {
protected:
	std::deque<std::shared_ptr<UimfFrame>> frames;
	std::condition_variable sig;
	std::mutex mut;

public:
	FrameSubscriber()
		: frames()
	{}

	void on_next(std::shared_ptr<UimfFrame> frame);
	int do_work(bool *should_exit);

private:
	virtual int execute() = 0;
};

#endif // !FRAME_SUBSCRIBER_H