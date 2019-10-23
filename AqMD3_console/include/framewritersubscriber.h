#ifndef FRAME_WRITER_SUBSCRIBER_H
#define FRAME_WRITER_SUBSCRIBER_H

#include "framesubscriber.h"
#include <UIMFWriter/UIMFWriter.h>
#include <deque>
#include <zmq.hpp>

class FrameWriterSubscriber : public FrameSubscriber<std::shared_ptr<UimfFrame>> {
private:

public:
	FrameWriterSubscriber() : FrameSubscriber()
	{}

private:
	void execute() override;
};

#endif // !FRAME_WRITER_SUBSCRIBER_H
