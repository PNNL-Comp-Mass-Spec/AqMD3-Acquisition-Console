#ifndef STREAMING_CONTEXT_H
#define STREAMING_CONTEXT_H

#include "acquireddata.h"
#include "AqMD3.h"

#include <memory>
#include <chrono>
#include <string>

class StreamingContext {
protected:
	ViSession session;
	AcquisitionBufferPool *samples;
	AcquisitionBufferPool *markers;
	std::string markersChannel;
	std::string samplesChannel;

public:
	StreamingContext(ViSession session, AcquisitionBufferPool *samples, AcquisitionBufferPool *markers,
		std::string markersChannel, std::string samplesChannel) :
			session(session),
			samples(samples),
			markers(markers),
			markersChannel(markersChannel),
			samplesChannel(samplesChannel)
	{}
	~StreamingContext() {
		delete samples, markers;
	}

	virtual AcquiredData acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) = 0;

	void start();
	void stop();
};

#endif // !STREAMING_CONTEXT_H
