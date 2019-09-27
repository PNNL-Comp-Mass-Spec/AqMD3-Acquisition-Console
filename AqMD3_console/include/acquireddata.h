#ifndef ACQUIRED_DATA_H
#define ACQUIRED_DATA_H

#include "acquisitionbufferpool.h"
#include "acquisitionbuffer.h"

#include <tuple>
#include <vector>

class TriggerData {
public:
	uint64_t timestamp;
	double subsample_pos;
	uint32_t index;

	// start, end, samples, processing elements
	std::vector<std::tuple<uint64_t, uint64_t, uint64_t, uint64_t>> gate_data;

	TriggerData(uint64_t timestamp, uint32_t index, double subsample_pos)
		: timestamp(timestamp)
		, index(index)
		, subsample_pos(subsample_pos)
		, gate_data()
	{}
};

class AcquiredData {
private:
	AcquisitionBufferPool *sample_buffer_pool;
	AcquisitionBuffer *sample_buffer;

	vector<TriggerData> stamps;
	uint64_t samples;

public:
	AcquiredData(vector<TriggerData> stamps, AcquisitionBufferPool *sample_pool, AcquisitionBuffer *sample_buffer, uint64_t samples)
		: sample_buffer_pool(sample_pool)
		, sample_buffer(sample_buffer)
		, stamps(stamps)
		, samples(samples)
	{}

	std::tuple<vector<uint64_t> const, vector<int32_t> const, vector<uint32_t> const> process() const;
};

#endif // !ACQUIRED_DATA_H
