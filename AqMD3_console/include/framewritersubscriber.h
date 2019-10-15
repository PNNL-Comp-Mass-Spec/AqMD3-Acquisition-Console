#ifndef FRAME_WRITER_SUBSCRIBER_H
#define FRAME_WRITER_SUBSCRIBER_H

#include "framesubscriber.h"
#include <UIMFWriter/UIMFWriter.h>
#include <deque>

class FrameWriterSubscriber : FrameSubscriber {
public:
	FrameWriterSubscriber()
	{}

private:
	int execute() override;
};

#endif // !FRAME_WRITER_SUBSCRIBER_H
