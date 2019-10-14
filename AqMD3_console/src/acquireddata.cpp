#include "../include/acquireddata.h"

#include <iostream>
#include <map>
#include <numeric>
#include <tuple>
#include <algorithm>

std::vector<EncodedResult> AcquiredData::process(int frame, int processing_scan_start_number) const
{
	std::vector<EncodedResult> results;
	results.reserve(stamps.size());
	
	int32_t *ptr = samples_buffer->get_raw_unprocessed();
	int offset = 0;

	for (int trig_index = 0; trig_index < stamps.size(); trig_index++)
	{
		auto trig = stamps[trig_index];
		auto timestamp = trig.timestamp;

		std::vector<int32_t> encoded_samples;

		uint64_t tic = 0;
		int64_t bpi = 0;
		double bpi_mz = 0.0;
		int32_t index_max_intensity = 0;
		
		int32_t non_zero_count = 0;
		int32_t zero_count = 0;

		size_t gate_count = trig.gate_data.size();
		for (int j = 0 ; j < gate_count; j++)
		{
			auto gate = trig.gate_data[j];
			auto samples = gate.get_gate_sample_length();
			auto elements = samples / 2;
			auto size_to_process = gate.processing_block_size;

			int32_t gate_zero_count = 0;
			if (gate_count > 1)
			{
				if (j == 0) 
				{
					gate_zero_count = gate.gate_start_index - 1;
				}
				else 
				{
					auto prev_gate = trig.gate_data[j - 1];
					gate_zero_count = prev_gate.get_sample_difference_next_gate(gate);
				}
			}
			else 
			{
				gate_zero_count =gate.gate_start_index - 1;
			}

			if (gate_zero_count != 0)
			{
				encoded_samples.push_back(-1 * gate_zero_count);
			}

			zero_count += gate_zero_count;

			int processed = 0;
			int32_t *ptr = samples_buffer->get_raw_unprocessed() + offset;
			for (int i = 0; i < elements; i++)
			{
				int32_t first = (ptr[i] << 16) >> 16;
				int32_t second = ptr[i] >> 16;

				// if(first < (zThreshold - zHysteresis))
				if (first < 0)
					first = 0;


				if (first > bpi)
				{
					bpi = first;
					index_max_intensity = non_zero_count + zero_count + (2 * i);
					bpi_mz = index_max_intensity;
				}

				encoded_samples.push_back(first);
				tic += first;

				if (second < 0)
					second = 0;

				if (second > bpi)
				{
					bpi = second;
					index_max_intensity = non_zero_count + zero_count + (2 * i) + 1;
					bpi_mz = index_max_intensity;
				}

				encoded_samples.push_back(second);
				tic += first + second;
				processed += 2;
			}
			offset += size_to_process;
			non_zero_count += samples;
		}

		results.emplace_back(
			frame,
			processing_scan_start_number + trig_index,
			non_zero_count,
			encoded_samples,
			tic,
			bpi,
			bpi_mz,
			index_max_intensity,
			timestamp);
	}
	
	return results;
}