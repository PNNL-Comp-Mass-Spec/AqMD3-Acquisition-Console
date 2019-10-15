#ifndef FRAME_PUBLISHER_H
#define FRAME_PUBLISHER_H

#include "framesubscriber.h"
#include <UIMFWriter/uimfframe.h>
#include <list>
#include <thread>

class FramePublisher {
private:
	std::list<std::unique_ptr<FrameSubscriber>> subscribers;

public:
	FramePublisher()
	{}

	void register_subscriber(std::unique_ptr<FrameSubscriber> subscriber) {
		subscribers.push_back(std::move(subscriber));
	}
};

#endif // !FRAME_PUBLISHER_H
