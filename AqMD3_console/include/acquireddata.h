#ifndef ACQUIRED_DATA_H
#define ACQUIRED_DATA_H

#include "acquisitionbuffer.h"
#include <UIMFWriter/encodedresult.h>

#include <tuple>
#include <vector>
#include <list>

class AcquiredData {
public:
	class GateData {
	public:
		uint64_t const gate_start_index;
		uint64_t const gate_stop_index;
		uint32_t const processing_block_size;

		GateData(uint64_t start_index, uint64_t stop_index, uint32_t memory_elements_int)
			: gate_start_index(start_index)
			, gate_stop_index(stop_index)
			, processing_block_size(memory_elements_int)
		{}

		inline uint64_t absolute_offset_from_start_index(uint64_t index) {
			if (index > gate_start_index)
				return index - gate_start_index;
			return gate_start_index - index;
		}

		inline uint64_t sample_difference_next_gate(const GateData& next) {
			return next.gate_start_index - gate_stop_index;
		}

		inline uint64_t gate_sample_length() {
			return gate_stop_index - gate_start_index;
		}
	};

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

private:
	std::shared_ptr<AcquisitionBuffer> samples_buffer;

public:
	std::vector<TriggerData> const stamps;
	uint64_t const samples_count;

public:
	AcquiredData(std::vector<TriggerData> stamps, std::shared_ptr<AcquisitionBuffer> samples_buffer, uint64_t samples_count)
		: samples_buffer(samples_buffer)
		, stamps(stamps)
		, samples_count(samples_count)
	{}

	std::vector<EncodedResult> AcquiredData::process(int frame, int processing_scan_start_number) const;
};

#endif // !ACQUIRED_DATA_H
