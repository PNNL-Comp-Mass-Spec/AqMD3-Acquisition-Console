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
	AcquisitionBufferPool markers_buffer_pool;

	int const gate_acquisition_multiplier = 2;
	int const marker_hunk_size = 16;
	int const max_target_records;
	int const min_target_records;

	AcquisitionBuffer* unprocessed_buffer;
	//std::vector<int32_t> markers_buffer;
public:
	CstZm1Context(ViSession session,
		std::string samples_channel,
		uint64_t samples_buffer_size,
		uint32_t samples_buffer_count,
		std::string markers_channel,
		uint64_t markers_size,
		uint32_t markers_count,
		uint64_t samples_per_trigger,
		uint32_t triggers_per_read)
		: StreamingContext(session, samples_channel, samples_buffer_size, samples_buffer_count, markers_channel, samples_per_trigger, triggers_per_read)
		, markers_buffer_pool(markers_size, markers_count)
		, residual(0)
		, add_residual(false)
		, unprocessed_buffer(nullptr)
		, min_target_records(triggers_per_read * marker_hunk_size)
		, max_target_records(triggers_per_read * marker_hunk_size * pow(gate_acquisition_multiplier, 3))
	{}

	AcquiredData acquire(std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_ZS1_CONTEXT_H
