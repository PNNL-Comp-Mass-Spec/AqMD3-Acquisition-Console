#ifndef PROCESS_SUBJECT_H
#define PROCESS_SUBJECT_H

#include "framesubscriber.h"
#include "framepublisher.h"
#include "acquireddata.h"
#include <UIMFWriter/encodedresult.h>
#include "server.h"
#include <zmq.hpp>

class ProcessSubject : public FrameSubscriber<AcquiredData>, public FramePublisher<std::shared_ptr<std::vector<EncodedResult>>> {
private:
	std::shared_ptr<UimfFrame> frame;
	int process_index;
	int total_triggers_processed;
	std::shared_ptr<Server::Publisher> publisher;
	std::string subject;

	std::chrono::milliseconds total_duration;
	int total_elements_processed;

	uint32_t offset_bins;

public:
	ProcessSubject(std::shared_ptr<UimfFrame> frame, std::shared_ptr<Server::Publisher> publisher)
		: frame(frame)
		, process_index(0)
		, total_triggers_processed(0)
		, publisher(publisher)
		, subject("status")
		, total_duration(0)
		, total_elements_processed(0)
		, offset_bins(0)
	{}

	ProcessSubject(uint32_t offset_bins)
		: process_index(0)
		, total_triggers_processed(0)
		, total_elements_processed(0)
		, total_duration(0)
		, offset_bins(offset_bins)
	{}

private:
	void on_notify() override;

	void on_completed() override
	{
		notify_completed_and_wait();
	}
};

#endif // !PROCESS_SUBJECT_H
