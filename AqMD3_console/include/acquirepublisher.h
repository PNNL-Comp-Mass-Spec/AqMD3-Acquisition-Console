#ifndef ACQUIRE_PUBLISHER_H
#define ACQUIRE_PUBLISHER_H

#include "framepublisher.h"
#include "acquisitioncontrol.h"
#include <vector>
#include <thread>

class AcquirePublisher : public AcquisitionControl, public FramePublisher<AcquiredData> {
private:
	std::thread worker_handle;
	std::unique_ptr<StreamingContext> context;

public:
	AcquirePublisher(std::unique_ptr<StreamingContext> context)
		: worker_handle()
		, context(std::move(context))
	{}

	void start() override;
	void stop() override;
};

#endif // !ACQUIRE_PUBLISHER_H
