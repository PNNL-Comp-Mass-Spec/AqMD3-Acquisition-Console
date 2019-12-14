#ifndef PROCESS_SUBJECT_H
#define PROCESS_SUBJECT_H

#include "framesubscriber.h"
#include "framepublisher.h"
#include "acquireddata.h"
#include <UIMFWriter/encodedresult.h>
#include "server.h"
#include <zmq.hpp>

using segment_ptr = std::shared_ptr<std::vector<EncodedResult>>;
using frame_ptr = std::shared_ptr<UimfFrame>;

class ProcessSubject : public FrameSubscriber<AcquiredData>, 
	public FramePublisher<segment_ptr>,
	public FramePublisher<frame_ptr> {
private:
	int total_triggers_processed;
	std::shared_ptr<Server::Publisher> publisher;
	std::string subject;
	std::chrono::milliseconds total_duration;
	int total_elements_processed;
	uint32_t offset_bins;
	uint64_t tof_avg_samples;
	std::deque<std::shared_ptr<UimfFrame>> frames;

public:
	ProcessSubject(std::shared_ptr<UimfFrame> frame, std::shared_ptr<Server::Publisher> publisher, uint64_t tof_avg_samples)
		: total_triggers_processed(0)
		, publisher(publisher)
		, subject("status")
		, total_duration(0)
		, total_elements_processed(0)
		, offset_bins(0)
		, tof_avg_samples(tof_avg_samples)
	{
		if (frame->nbr_accumulations == 1)
		{
			frames.push_back(frame->clone_at_frame(frame->frame_number));
		}
		else
		{
			for (int i = 0; i < frame->nbr_accumulations; i++)
			{
				frames.push_back(frame->clone_at_frame(i));
			}
		}
	}

	ProcessSubject(uint32_t offset_bins, uint64_t tof_avg_samples)
		: total_triggers_processed(0)
		, total_elements_processed(0)
		, total_duration(0)
		, offset_bins(offset_bins)
		, tof_avg_samples(tof_avg_samples)
	{}

private:
	void on_notify() override;

	void on_completed() override
	{
		FramePublisher<segment_ptr>::notify_completed_and_wait();
	}
};

#endif // !PROCESS_SUBJECT_H
