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
	private:
		int const processing_block_size = 8;

	public:
		uint64_t const gate_start_block_index;
		uint64_t const gate_stop_block_index;

		uint32_t const gate_start_intra_block_index;
		uint32_t const gate_stop_intra_block_index;

		uint32_t const total_processing_blocks;

		GateData(uint64_t gate_start_block_index, uint32_t gate_start_intra_block_index, uint64_t gate_stop_block_index,
			uint32_t gate_stop_intra_block_index,
			uint32_t total_processing_blocks)
			: gate_start_block_index(gate_start_block_index)
			, gate_start_intra_block_index(gate_start_intra_block_index)
			, gate_stop_block_index(gate_stop_block_index)
			, gate_stop_intra_block_index(gate_stop_intra_block_index)
			, total_processing_blocks(total_processing_blocks)
		{}

		inline uint64_t get_start_sample_index() const {
			return (gate_start_block_index - 1) * processing_block_size + gate_start_intra_block_index;
		}

		inline uint64_t get_stop_sample_index() const {
			return (gate_stop_block_index - 1) * processing_block_size - (processing_block_size - gate_stop_intra_block_index);
		}

	};

	class TriggerData {
	public:
		uint64_t const timestamp;
		double const subsample_pos;
		uint32_t const index;

		std::vector<GateData> gate_data;

		TriggerData(uint64_t timestamp, uint32_t index, double subsample_pos)
			: timestamp(timestamp)
			, index(index)
			, subsample_pos(subsample_pos)
			, gate_data()
		{}
	};

private:
	std::shared_ptr<AcquisitionBuffer> samples_buffer;
	int32_t const range_shift_constant = 32768; // [-2^15, 2^15 - 1] + 2^15 -> [0, 2^16 - 1]

public:
	std::vector<TriggerData> const stamps;
	uint64_t const samples_count;

public:
	AcquiredData(std::vector<TriggerData> stamps, std::shared_ptr<AcquisitionBuffer> samples_buffer, uint64_t samples_count)
		: samples_buffer(samples_buffer)
		, stamps(stamps)
		, samples_count(samples_count)
	{}

	std::shared_ptr<std::vector<EncodedResult>> AcquiredData::process(int processing_scan_start_number, uint32_t post_trigger_samples) const;

};

#endif // !ACQUIRED_DATA_H
