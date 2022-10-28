#ifndef CST_CONTEXT_H
#define CST_CONTEXT_H

#include "streamingcontext.h"

class CstContext : public StreamingContext {
private:

public:
	CstContext(const Digitizer& digitizer, std::string channel, int32_t samples_buffer_count,
		uint64_t samples_per_trigger,
		uint32_t triggers_per_read)
		: StreamingContext(digitizer, channel, samples_buffer_count, samples_per_trigger, triggers_per_read)
	{}

	AcquiredData acquire(std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_CONTEXT_H
