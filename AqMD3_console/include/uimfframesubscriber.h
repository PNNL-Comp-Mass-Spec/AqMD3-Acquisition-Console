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

	std::deque<std::shared_ptr<UimfFrame>> frames;

public:
	UimfFrameSubscriber(std::shared_ptr<UimfFrame> frame)
		: frame(frame)
		, total_processed(0)
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
	void execute() override;

	void on_completed() override
	{
		execute();
	}
};

#endif // !UIMF_FRAME_SUBSCRIBER_H
