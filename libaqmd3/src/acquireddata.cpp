#include "../include/libaqmd3/acquireddata.h"

#include <iostream>
#include <map>
#include <numeric>
#include <tuple>
#include <algorithm>

std::shared_ptr<std::vector<EncodedResult>> AcquiredData::process(int processing_scan_start_number, uint32_t post_trigger_samples) const
{
	std::shared_ptr<std::vector<EncodedResult>> results = std::make_shared<std::vector<EncodedResult>>();
	results->reserve(stamps.size());

	int32_t offset = 0;

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

		int gate_count = trig.gate_data.size();
		
		// try reserve
		int reserve = 0;
		for (int k = 0; k < gate_count; k++)
		{
			const auto gate = &trig.gate_data[k];
			int64_t samples = int64_t(gate->get_stop_sample_index()) - int64_t(gate->get_start_sample_index());
			if (samples > 0)
			{
				reserve += samples;
			}
		}
		encoded_samples.reserve(reserve + gate_count);

		for (int j = 0; j < gate_count; j++)
		{
			const auto& gate = &trig.gate_data[j];
			auto elements = gate->element_count;
			
			auto samples = int64_t(gate->get_stop_sample_index()) - int64_t(gate->get_start_sample_index());

			if (gate->get_stop_sample_index() <= gate->get_start_sample_index())
			{
				offset += elements;
				continue;
			}

#pragma region zero_count
			int32_t gate_zero_count = 0;
			if (j == 0)
			{
				gate_zero_count = gate->get_start_sample_index() + post_trigger_samples;
			}
			else
			{
				auto prev_gate = &trig.gate_data[j - 1];
				gate_zero_count = gate->get_start_sample_index() - prev_gate->get_stop_sample_index();
			}

			if (gate_zero_count > 0)
			{
				encoded_samples.push_back(-1 * gate_zero_count);
			}

			zero_count += gate_zero_count;
#pragma endregion

			int32_t *ptr = samples_buffer->get_raw_unprocessed() + offset;

			for (int i = gate->gate_start_sample_pos; i < gate->gate_start_sample_pos + samples; i++)
			{
				int32_t val = 0;
				int data_index = i / 2;

				if (i % 2 == 0)
					val = ((ptr[data_index] << 16) >> 16) + range_shift_constant;
				else
					val = (ptr[data_index] >> 16) + range_shift_constant;
				
				if (val > bpi)
				{
					bpi = val;
					index_max_intensity = non_zero_count + zero_count;
					bpi_mz = index_max_intensity;
				}

				encoded_samples.push_back(val);
				non_zero_count++;
				tic += val;
			}

			offset += elements;
		}
		results->emplace_back(
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
