#ifndef CST_ZS1_CONTEXT_H
#define CST_ZS1_CONTEXT_H

#include "streamingcontext.h"

class CstZm1Context : public StreamingContext {
public:
	CstZm1Context(ViSession* session, AcquisitionBufferPool &data, AcquisitionBufferPool &markers,
		std::string markersChannel, std::string samplesChannel)
		: StreamingContext(session, data, markers, markersChannel, samplesChannel)
	{}

	std::unique_ptr<AcquiredData> acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_ZS1_CONTEXT_H
