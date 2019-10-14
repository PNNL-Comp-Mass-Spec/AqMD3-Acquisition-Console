#ifndef FRAME_PUBLISHER_H
#define FRAME_PUBLISHER_H

#include "framesubscriber.h"
#include <UIMFWriter/uimfframe.h>

class FramePublisher {
public:
	FramePublisher()
	{}

	void register_subscriber(FrameSubscriber<UimfFrame> subscriber);
};

#endif // !FRAME_PUBLISHER_H
