#ifndef FRAME_SUBJECT_H
#define FRAME_SUBJECT_H

#include "framepublisher.h"
#include "framesubscriber.h"
#include <vector>
#include <UIMFWriter/uimfframe.h>
#include <thread>
#include <zmq.hpp>

class FrameSubject : public FramePublisher<std::shared_ptr<UimfFrame>>,
	public FrameSubscriber<AcquiredData>
{
private:
	std::thread worker_handle;
	std::shared_ptr<UimfFrame> frame;
	zmq::socket_t socket;
	std::string subject;
	int process_index;
	int total_processed;

public:
	FrameSubject(std::shared_ptr<UimfFrame> frame, zmq::socket_t socket) : FramePublisher(), FrameSubscriber()
		, worker_handle()
		, socket(std::move(socket))
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
