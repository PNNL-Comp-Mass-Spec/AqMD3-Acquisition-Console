#ifndef CST_CONTEXT_H
#define CST_CONTEXT_H

#include "streamingcontext.h"

class CstContext : public StreamingContext {
private:

public:
	CstContext(ViSession session, std::string samples_channel, int64_t samples_buffer_size, int32_t samples_buffer_count,
		std::string markers_channel,
		uint64_t samples_per_trigger,
		uint32_t triggers_per_read)
		: StreamingContext(session, samples_channel, samples_buffer_size, samples_buffer_count, markers_channel, samples_per_trigger, triggers_per_read)
	{}

	AcquiredData acquire(std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_CONTEXT_H
