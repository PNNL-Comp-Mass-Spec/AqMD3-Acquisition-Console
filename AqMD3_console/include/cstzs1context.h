#ifndef CST_ZS1_CONTEXT_H
#define CST_ZS1_CONTEXT_H

#include "streamingcontext.h"

#include <vector>
#include <tuple>
#include <algorithm>

class CstZm1Context : public StreamingContext {
private:
	int const gate_acquisition_multiplier = 2;
	int const markers_hunk_size = 16;
	int const min_target_records;

	int active_multiplier;

	int const multiplier_min = 1;
	int const multiplier_max = 8;

	AcquisitionBuffer markers_buffer;

public:
	CstZm1Context(Digitizer& digitizer,
		std::string channel,
		uint64_t samples_buffer_size,
		uint32_t samples_buffer_count,
		uint64_t samples_per_trigger,
		uint32_t triggers_per_read)
		: StreamingContext(digitizer, channel, samples_buffer_size, samples_buffer_count, samples_per_trigger, triggers_per_read)
		, min_target_records(triggers_per_read * markers_hunk_size)
		, markers_buffer(0, (triggers_per_read * markers_hunk_size * multiplier_max) + 15)
		, active_multiplier(1)
	{}

	AcquiredData acquire(std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_ZS1_CONTEXT_H
