#ifndef STREAMING_CONTEXT_H
#define STREAMING_CONTEXT_H

#include "acquireddata.h"
#include "AqMD3.h"

#include <chrono>

class StreamingContext {
private:
	ViSession session;
	AcquisitionBufferPool data;
	AcquisitionBufferPool markers;
public:
	StreamingContext(ViSession& session, AcquisitionBufferPool &data, AcquisitionBufferPool &markers)
		: session(session), data(data), markers(markers)
	{}

	virtual AcquiredData* acquire(ViConstString dataStream, ViConstString markerStream, int32_t triggers,
		std::chrono::milliseconds timeoutMs) = 0;

	void start();
	void stop();
};

#endif // !STREAMING_CONTEXT_H
