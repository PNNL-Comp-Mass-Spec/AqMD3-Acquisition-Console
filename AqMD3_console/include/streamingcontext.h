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

	//StreamingContext(ViSession session, int64_t data_buffer_size, int32_t data_buffer_count,
	//	int64_t markers_buffer_size,
	//	int32_t markers_buffer_count,
	//  std::string markers_channel, std::string samples_channel)
	//{}

	virtual AcquiredData acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) = 0;

	void start();
	void stop();
};

#endif // !STREAMING_CONTEXT_H
