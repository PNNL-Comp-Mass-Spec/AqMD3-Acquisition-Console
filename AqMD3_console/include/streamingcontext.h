#ifndef STREAMING_CONTEXT_H
#define STREAMING_CONTEXT_H

#include "acquireddata.h"
#include "AqMD3.h"

#include <chrono>
#include <string>

class StreamingContext {
private:
	ViSession* session;
	AcquisitionBufferPool data;
	AcquisitionBufferPool markers;
	std::string markersChannel;
	std::string samplesChannel;

public:
	StreamingContext(ViSession* session, AcquisitionBufferPool &data, AcquisitionBufferPool &markers,
		std::string markersChannel, std::string samplesChannel) :
			session(session),
			data(data),
			markers(markers),
			markersChannel(markersChannel),
			samplesChannel(samplesChannel)
	{}
	~StreamingContext()
	{
		delete session;
	}

	virtual std::unique_ptr<AcquiredData> acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) = 0;

	void start();
	void stop();
};

#endif // !STREAMING_CONTEXT_H
