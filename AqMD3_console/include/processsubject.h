#ifndef PROCESS_SUBJECT_H
#define PROCESS_SUBJECT_H

#include "framesubscriber.h"
#include "framepublisher.h"
#include "acquireddata.h"
#include <UIMFWriter/encodedresult.h>
#include "server.h"
#include <zmq.hpp>

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

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
	uint64_t total_elements_processed;
	uint32_t offset_bins;
	uint64_t tof_avg_samples;
	std::deque<std::shared_ptr<UimfFrame>> frames;

public:
	ProcessSubject(UimfRequestMessage uimf, std::shared_ptr<Server::Publisher> publisher, uint32_t offset_bins, uint64_t tof_avg_samples)
		: total_triggers_processed(0)
		, publisher(publisher)
		, subject("status")
		, total_duration(0)
		, total_elements_processed(0)
		, offset_bins(offset_bins)
		, tof_avg_samples(tof_avg_samples)
	{
		std::shared_ptr<UimfFrame> frame = std::make_shared<UimfFrame>(
			uimf.start_trigger(),
			uimf.nbr_samples(),
			uimf.nbr_accumulations(),
			uimf.frame_length(),
			uimf.frame_number(),
			uimf.offset_bins(),
			uimf.file_name()
			);

		if (frame->nbr_accumulations == 1)
		{
			frames.push_back(frame);
		}
		else
		{
			for (int i = 0; i < frame->nbr_accumulations; i++)
			{
				frames.push_back(frame->clone(i));
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
		FramePublisher<frame_ptr>::notify_completed_and_wait();
	}
};

#endif // !PROCESS_SUBJECT_H
