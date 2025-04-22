#ifndef CST_CONTEXT_H
#define CST_CONTEXT_H

#include "streamingcontext.h"

class CstContext : public StreamingContext {
private:

public:
	CstContext(const Digitizer& digitizer, std::string channel, std::shared_ptr<AcquisitionBufferPool> buffer_pool)
		: StreamingContext(digitizer, channel, buffer_pool)
	{}

	AcquiredData acquire(uint64_t triggers_to_read, std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_CONTEXT_H
