#ifndef ACQUIRE_PUBLISHER_H
#define ACQUIRE_PUBLISHER_H

#include "publisher.h"
#include "acquisitioncontrol.h"
#include <libaqmd3/streamingcontext.h>
#include <vector>
#include <thread>
#include <atomic>

class AcquirePublisher : public AcquisitionControl, public Publisher<AcquiredData> {
private:
	std::thread worker_handle;
	std::shared_ptr<StreamingContext> context;
	std::atomic_bool should_stop;

	std::promise<State> stop_signal;

public:
	AcquirePublisher(std::shared_ptr<StreamingContext> context)
		: worker_handle()
		, context(context)
		, should_stop(false)
	{}
	virtual ~AcquirePublisher() = default;

	void start(UimfRequestMessage uimf);
	void start() override;
	void stop() override;

};

#endif // !ACQUIRE_PUBLISHER_H
