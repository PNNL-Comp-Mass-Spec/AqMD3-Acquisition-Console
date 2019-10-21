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
			auto blocks = gate.total_processing_blocks;
			auto samples = blocks * 2;
			auto first_valid_index = gate.gate_start_intra_block_index;
			auto last_valid_index = gate.get_stop_sample_index() - gate.get_start_sample_index();

			int32_t gate_zero_count = 0;
			if (j == 0)
			{
				gate_zero_count = gate.get_start_sample_index();
			}
			else 
			{
				auto prev_gate = trig.gate_data[j - 1];
				gate_zero_count = gate.get_start_sample_index() - prev_gate.get_stop_sample_index();
			}

			if (gate_zero_count != 0)
			{
				encoded_samples.push_back(-1 * gate_zero_count);
			}

			zero_count += gate_zero_count;

			int processed = 0;
			int32_t *ptr = samples_buffer->get_raw_unprocessed() + offset;
			
			int start = first_valid_index / 2;
			if (first_valid_index % 2 != 0)
			{
				auto val = (ptr[start] >> 16);
				// if(first < (zThreshold - zHysteresis))
				if (val < 0)
					val = 0;

				if (val > bpi)
				{
					bpi = val;
					index_max_intensity = non_zero_count + zero_count + (2 * start);
					bpi_mz = index_max_intensity;
				}

				encoded_samples.push_back(val);
				tic += val;

				start++;
			}

			for (int i = start; i < blocks; i++)
			{
				for (auto val : { ((ptr[i] << 16) >> 16) , (ptr[i] >> 16) })
				{
					// if(first < (zThreshold - zHysteresis))
					if (val < 0)
						val = 0;

					if (val > bpi)
					{
						bpi = val;
						index_max_intensity = non_zero_count + zero_count + (2 * i);
						bpi_mz = index_max_intensity;
					}

					encoded_samples.push_back(val);
					tic += val;
				}
				processed += 2;
			}
			offset += blocks;
			//non_zero_count += samples;
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