#ifndef CST_CONTEXT_H
#define CST_CONTEXT_H

#include "streamingcontext.h"

class CstContext/* : public StreamingContext */ {
public:
	CstContext(ViSession session, AcquisitionBufferPool *samples, AcquisitionBufferPool *markers,
		std::string markersChannel, std::string samplesChannel)
		//: StreamingContext(session, samples, markers, markersChannel, samplesChannel)
	{}

	//AcquiredData acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_CONTEXT_H
