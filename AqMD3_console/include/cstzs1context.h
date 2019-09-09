#ifndef CST_ZS1_CONTEXT_H
#define CST_ZS1_CONTEXT_H

#include "streamingcontext.h"

#include <vector>
#include <tuple>
#include <algorithm>
#include <iostream>

class CstZm1Context : public StreamingContext {
private:
	int unprocessed;
	AcquisitionBuffer* unprocessed_buf;

	int markers_overhead;
	int samples_overhead;
public:
	CstZm1Context(ViSession session, AcquisitionBufferPool* data, AcquisitionBufferPool* markers,
		std::string markersChannel, std::string samplesChannel, int markers_overhead_grain, int samples_overhead_grain)
		: StreamingContext(session, data, markers, markersChannel, samplesChannel)
		, unprocessed(0)
		, unprocessed_buf(nullptr)
		, markers_overhead(markers_overhead_grain / sizeof(int32_t) - 1)
		, samples_overhead(samples_overhead_grain / sizeof(int32_t) - 1)
	{
		std::cout << "markers overhead: " << markers_overhead << std::endl;
		std::cout << "samples overhead: " << samples_overhead << std::endl;
	}

	AcquiredData acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_ZS1_CONTEXT_H
