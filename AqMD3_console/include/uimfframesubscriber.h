#ifndef UIMF_FRAME_SUBSCRIBER_H
#define UIMF_FRAME_SUBSCRIBER_H

#include "framepublisher.h"
#include "framesubscriber.h"
#include <vector>
#include <UIMFWriter/uimfframe.h>
#include <thread>
#include <deque>
#include <iostream>


class UimfFrameSubscriber : public FrameSubscriber<std::shared_ptr<std::vector<EncodedResult>>> {
private:
	std::shared_ptr<UimfFrame> frame;
	int total_processed;
	double ts_sample_period;

	std::deque<std::shared_ptr<UimfFrame>> frames;

public:
	UimfFrameSubscriber(std::shared_ptr<UimfFrame> frame, double ts_sample_period)
		: frame(frame)
		, total_processed(0)
		, ts_sample_period(ts_sample_period)
	{
		if (this->frame->nbr_accumulations == 1)
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

private:
	void on_notify() override;

	void on_completed() override
	{
		on_notify();
	}
};

#endif // !UIMF_FRAME_SUBSCRIBER_H
