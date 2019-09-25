#ifndef CST_ZS1_CONTEXT_H
#define CST_ZS1_CONTEXT_H

#include "streamingcontext.h"

#include <vector>
#include <tuple>
#include <algorithm>
#include <iostream>

class CstZm1Context : public StreamingContext {
private:
	int residual;
	bool add_residual;

	AcquisitionBuffer* unprocessed_buf;

	int markers_overhead;
	int samples_overhead;
public:
	CstZm1Context(ViSession session, AcquisitionBufferPool* data, AcquisitionBufferPool* markers,
		std::string markersChannel, std::string samplesChannel, int markers_overhead_grain, int samples_overhead_grain)
		: StreamingContext(session, data, markers, markersChannel, samplesChannel)
		, unprocessed_buf(nullptr)
		, markers_overhead(markers_overhead_grain / sizeof(int32_t) - 1)
		, samples_overhead(samples_overhead_grain / sizeof(int32_t) - 1)
		, residual(0)
		, add_residual(false)
	{
		//std::cout << "markers overhead: " << markers_overhead << std::endl;
		//std::cout << "samples overhead: " << samples_overhead << std::endl;
	}

	//CstZm1Context(ViSession session, int64_t data_buffer_size, int32_t data_buffer_count,
	//	int64_t markers_buffer_size,
	//	int32_t markers_buffer_count,
	//  std::string markers_channel, std::string samples_channel)
	//  : StreamingContext(session, data_buffer_size, data_buffer_count, 
	//  markers_buffer_size, markers_buffer_count, markers_channel, samples_channel)
	//{}

	AcquiredData acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_ZS1_CONTEXT_H
