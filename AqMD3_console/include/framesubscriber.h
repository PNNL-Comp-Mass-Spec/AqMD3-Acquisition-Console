#ifndef FRAME_SUBSCRIBER_H
#define FRAME_SUBSCRIBER_H

#include <UIMFWriter/uimfframe.h>

template <typename T>
class FrameSubscriber {
public:
	FrameSubscriber()
	{}

	void on_next(T frame);
};

#endif // !FRAME_SUBSCRIBER_H
