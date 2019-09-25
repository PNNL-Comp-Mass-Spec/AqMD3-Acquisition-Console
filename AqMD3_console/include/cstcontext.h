#ifndef CST_CONTEXT_H
#define CST_CONTEXT_H

#include "streamingcontext.h"

class CstContext : public StreamingContext {
private:
	int64_t samples_per_trigger;

public:
	CstContext(ViSession session, AcquisitionBufferPool *samples, AcquisitionBufferPool *markers,
		std::string markersChannel, std::string samplesChannel, int64_t samplesPerTrigger)
		: StreamingContext(session, samples, markers, markersChannel, samplesChannel)
		, samples_per_trigger(samplesPerTrigger)
	{}

	AcquiredData acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_CONTEXT_H
