#ifndef FRAME_SUBJECT_H
#define FRAME_SUBJECT_H

#include "framepublisher.h"
#include "framesubscriber.h"
#include "server.h"
#include <vector>
#include <UIMFWriter/uimfframe.h>
#include <thread>
#include <zmq.hpp>

#include <iostream>

class FrameSubject : public FramePublisher<std::shared_ptr<UimfFrame>>, public FrameSubscriber<AcquiredData>
{
private:
	std::shared_ptr<UimfFrame> frame;
	std::shared_ptr<Server::Publisher> publisher;

	std::string subject;
	int process_index;
	int total_processed;

public:
	FrameSubject(std::shared_ptr<UimfFrame> frame, std::shared_ptr<Server::Publisher> publisher)
		: FramePublisher()
		, FrameSubscriber()
		, publisher(publisher)
		, subject("status")
		, frame(std::move(frame))
		, process_index(0)
		, total_processed(0)
	{
	}

private:
	void execute() override;

};

#endif // !FRAME_SUBJECT_H
