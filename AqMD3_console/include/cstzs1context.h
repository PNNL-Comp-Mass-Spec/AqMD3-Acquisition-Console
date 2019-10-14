#ifndef CST_ZS1_CONTEXT_H
#define CST_ZS1_CONTEXT_H

#include "streamingcontext.h"

#include <vector>
#include <tuple>
#include <algorithm>
#include <iostream>

class CstZm1Context : public StreamingContext {
private:
	int const gate_acquisition_multiplier = 2;
	int const markers_hunk_size = 16;
	int const max_target_records;
	int const min_target_records;

	AcquisitionBuffer markers_buffer;

public:
	CstZm1Context(ViSession session,
		std::string channel,
		uint64_t samples_buffer_size,
		uint32_t samples_buffer_count,
		uint64_t samples_per_trigger,
		uint32_t triggers_per_read)
		: StreamingContext(session, channel, samples_buffer_size, samples_buffer_count, samples_per_trigger, triggers_per_read)
		, min_target_records(triggers_per_read * markers_hunk_size)
		, max_target_records(triggers_per_read * markers_hunk_size * pow(gate_acquisition_multiplier, 3))
		, markers_buffer(0, max_target_records + 15)
	{}

	AcquiredData acquire(std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_ZS1_CONTEXT_H
