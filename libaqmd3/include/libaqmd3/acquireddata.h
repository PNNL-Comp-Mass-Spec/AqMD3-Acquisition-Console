#ifndef ACQUIRED_DATA_H
#define ACQUIRED_DATA_H

#include "acquisitionbuffer.h"
#include <UIMFWriter/encodedresult.h>
#include <tuple>
#include <vector>
#include <list>

// Contains raw sample data, timestamp data, and gated-record data for both CST and CST-ZS acquisition.
// For CST acquision, this class simply holds the raw sample data and cannot be used to extract samples
// from elements.
class AcquiredData {
public:
	// Gated record descriptor class used to calculate the first and last valid sample index, and the number of elements to process in the 
	// flat array of elements.
	class GateData {
	private:
		// This is the minimum size of a gated record on the SA220P and is used to calculate the sample index of the 
		// first valid sample in a gated record and the position of the gated record in the the area after a trigger
		// event when a sample can be recorded (also known as the "record").
		int const processing_block_size = 8;

	public:
		// *_block_index member variables are the index in units of processing_block_size blocks of int32 elements within the record.
		uint64_t const gate_start_block_index;
		uint64_t const gate_stop_block_index;

		// *_sample_pos member variables are the index of the first valid sample or one after the last valid sample within the processing_block_size 
		// sized block at index gate_*_block index.
		uint32_t const gate_start_sample_pos;
		uint32_t const gate_stop_sample_pos;

		// The total number of elements in the gated record. 
		uint32_t const element_count;

		GateData(uint64_t gate_start_block_index, uint32_t gate_start_sample_pos, uint64_t gate_stop_block_index,
			uint32_t gate_stop_sample_pos,
			uint32_t element_count)
			: gate_start_block_index(gate_start_block_index)
			, gate_start_sample_pos(gate_start_sample_pos)
			, gate_stop_block_index(gate_stop_block_index)
			, gate_stop_sample_pos(gate_stop_sample_pos)
			, element_count(element_count)
		{}

		inline uint64_t get_start_sample_index() const {
			return (gate_start_block_index - 1) * processing_block_size + gate_start_sample_pos;
		}

		inline uint64_t get_stop_sample_index() const {
			return (gate_stop_block_index - 1) * processing_block_size - (processing_block_size - gate_stop_sample_pos);
		}

	};

	// Couples trigger timestamps data with gated record information.
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
	
	// Used to bring data into the unsigned int16 range. [-2^15, 2^15 - 1] + 2^15 -> [0, 2^16 - 1]
	int32_t const range_shift_constant = 32768;

public:
	std::vector<TriggerData> const stamps;
	uint64_t const samples_count;

public:
	AcquiredData(std::vector<TriggerData> stamps, std::shared_ptr<AcquisitionBuffer> samples_buffer, uint64_t samples_count)
		: samples_buffer(samples_buffer)
		, stamps(stamps)
		, samples_count(samples_count)
	{}

	// Processes the raw sample data and returns a vector of processed results equivalent to a row of scan data in a .uimf file. 
	// Should not be called if the data was acquired without data-reduction enabled.
	std::shared_ptr<std::vector<EncodedResult>> process(int processing_scan_start_number, uint32_t post_trigger_samples) const;

};

#endif // !ACQUIRED_DATA_H
