#ifndef ACQUIRE_FRAME_PUBLISHER_H
#define ACQUIRE_FRAME_PUBLISHER_H

#include "framepublisher.h"
#include "acquisitioncontrol.h"
#include <vector>
#include <thread>
#include <UIMFWriter/uimfframe.h>

class AcquireFramePublisher : public AcquisitionControl, public FramePublisher<AcquiredData> {
private:
	std::thread worker_handle;
	std::unique_ptr<StreamingContext> context;
	std::shared_ptr<UimfFrame> frame;
	std::atomic_bool should_stop;
	int total_triggers;

public:
	AcquireFramePublisher(std::unique_ptr<StreamingContext> context, std::shared_ptr<UimfFrame> frame)
		: worker_handle()
		, context(std::move(context))
		, frame(frame)
		, total_triggers(frame->frame_length * frame->nbr_accumulations)
		, should_stop(false)
	{}

	void start() override;
	void stop() override;
};

#endif // !ACQUIRE_FRAME_PUBLISHER_H
