#ifndef ACQUIRE_PUBLISHER_H
#define ACQUIRE_PUBLISHER_H

#include "publisher.h"
#include "acquisitioncontrol.h"
#include "uimfacquisitionrecord.h"
#include "server.h"
#include <libaqmd3/streamingcontext.h>
#include <vector>
#include <thread>
#include <atomic>

class AcquirePublisher : public AcquisitionControl, public Publisher<UimfAcquisitionRecord> {
private:
	std::unique_ptr<std::thread> worker_handle;//<
	std::shared_ptr<StreamingContext> digitizer;
	std::atomic_bool should_stop;//<
	std::promise<State> stop_signal;//<
	int64_t timeout;
	std::shared_ptr<AcquisitionBufferPool> buffer_pool;
	uint64_t segment_size;
	std::shared_ptr<Server::Publisher> publisher;
	std::string subject;

public:
	AcquirePublisher(std::shared_ptr<StreamingContext> digitizer, int64_t timeout, std::shared_ptr<AcquisitionBufferPool> buffer_pool,
		uint64_t segment_size,
		std::shared_ptr<Server::Publisher> publisher)
		: worker_handle()
		, digitizer(digitizer)
		, should_stop(false)
		, timeout(timeout)
		, buffer_pool(buffer_pool)
		, segment_size(segment_size)
		, publisher(publisher)
		, subject("status")
	{}
	virtual ~AcquirePublisher() = default;

	void start(UimfFrameParameters parameters);
	//void start() override;
	void stop(bool terminate_acquisition_chain) override;

};

#endif // !ACQUIRE_PUBLISHER_H
