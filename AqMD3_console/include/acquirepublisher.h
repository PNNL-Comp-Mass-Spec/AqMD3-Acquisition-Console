#ifndef ACQUIRE_PUBLISHER_H
#define ACQUIRE_PUBLISHER_H

#include "framepublisher.h"
#include "acquisitioncontrol.h"
#include <vector>
#include <thread>
#include <atomic>
#include <iostream>

class AcquirePublisher : public AcquisitionControl, public FramePublisher<AcquiredData> {
private:
	std::thread worker_handle;
	std::unique_ptr<StreamingContext> context;
	std::atomic_bool should_stop;

	std::condition_variable stop_sig;
	std::mutex stop_mut;

public:
	AcquirePublisher(std::unique_ptr<StreamingContext> context)
		: worker_handle()
		, context(std::move(context))
		, should_stop(false)
	{}
	virtual ~AcquirePublisher() = default;

	void start() override;
	void stop() override;

};

#endif // !ACQUIRE_PUBLISHER_H
