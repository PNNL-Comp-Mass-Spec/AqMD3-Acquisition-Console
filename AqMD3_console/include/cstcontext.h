#ifndef CST_CONTEXT_H
#define CST_CONTEXT_H

#include "streamingcontext.h"

class CstContext : public StreamingContext {
public:
	CstContext(ViSession& session, AcquisitionBufferPool &data, AcquisitionBufferPool &markers)
		: StreamingContext(session, data, markers)
	{}

	AcquiredData* acquire(ViConstString dataStream, ViConstString markerStream, int32_t triggers,
		std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_CONTEXT_H
