#ifndef UIMF_FRAME_WRITER_SUBSCRIBER_H
#define UIMF_FRAME_WRITER_SUBSCRIBER_H

#include "framesubscriber.h"
#include <UIMFWriter/uimfframe.h>


class UimfFrameWriterSubscriber : public FrameSubscriber<std::shared_ptr<UimfFrame>> {
private:
	std::deque<std::shared_ptr<UimfFrame>> frames;
	bool append_excess;

public:
	UimfFrameWriterSubscriber()
		: FrameSubscriber(true)
	{}

private:
	void on_notify() override;
	void on_completed() override;
};

#endif // !UIMF_FRAME_WRITER_SUBSCRIBER_H
