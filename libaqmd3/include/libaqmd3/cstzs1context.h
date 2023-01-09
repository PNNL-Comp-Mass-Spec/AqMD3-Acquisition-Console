#ifndef CST_ZS1_CONTEXT_H
#define CST_ZS1_CONTEXT_H

#include "streamingcontext.h"

#include <vector>
#include <tuple>
#include <algorithm>

class CstZs1Context : public StreamingContext {
private:
	uint64_t const gate_acquisition_multiplier = 2;
	uint64_t const markers_hunk_size = 16;
	uint64_t const min_target_records;

	int active_multiplier;

	int const multiplier_min = 1;
	int const multiplier_max = 8;

	//TODO use array or vector
	AcquisitionBuffer markers_buffer;

public:
	CstZs1Context(const Digitizer& digitizer, std::string channel, uint64_t triggers_per_read, std::shared_ptr<AcquisitionBufferPool> buffer_pool)
		: StreamingContext(digitizer, channel, buffer_pool)
		, min_target_records(triggers_per_read * markers_hunk_size)
		, markers_buffer((size_t)(triggers_per_read * markers_hunk_size * multiplier_max) + 15)
		, active_multiplier(1)
	{}

	void stop() override;
	AcquiredData acquire(std::chrono::milliseconds timeoutMs) override;
};

#endif // !CST_ZS1_CONTEXT_H
