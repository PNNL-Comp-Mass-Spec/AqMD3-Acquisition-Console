#include "../include/libaqmd3/cstzs1context.h"
#include "../include/libaqmd3/digitizer.h"

#include <vector>
#include <tuple>
#include <exception>
#include <fstream>
#include <iostream>
using std::cout;
using std::cerr;
#include <limits>
#include <stdexcept>
#include <sstream>

#define variable_m_t_a

void CstZs1Context::stop()
{
	StreamingContext::stop();
	markers_buffer.reset();
}

AcquiredData CstZs1Context::acquire(std::chrono::milliseconds timeoutMs)
{
	int markers_to_acquire = min_target_records;
	int active_multiplier = 1;

	int trig_count = 0;
	int gate_count = 0;
	uint64_t to_acquire = 0;
	std::vector<AcquiredData::TriggerData> stamps;

	std::shared_ptr<AcquisitionBuffer> samples_buffer = get_buffer();

	ViInt64 first_element_markers;
	ViInt64 available_elements_markers = 0;
	ViInt64 actual_elements_markers = markers_buffer.get_unprocessed();

	bool use_timeout = (timeoutMs == std::chrono::milliseconds::zero()) ? false : true;

	// sanity check
	if (actual_elements_markers % 16 != 0)
	{
		throw std::runtime_error("actual_elements_markers % 16 != 0 at start of acquire");
	}

	auto finish = std::chrono::high_resolution_clock::now() + timeoutMs;
	while (trig_count <= triggers_per_read)
	{

		for (int i = 0; i < actual_elements_markers / 16; i++)
		{
			int32_t *seg = markers_buffer.get_raw_unprocessed();
			uint32_t header = seg[0];

			switch (header & 0x000000FF)
			{
			case 0x01:
			{
				++trig_count;
				if (trig_count >= triggers_per_read + 1)
					goto process;

				uint64_t low = seg[1];
				uint64_t high = seg[2];
				uint64_t timestampLow = (low >> 8) & 0x0000000000ffffffL;
				uint64_t timestampHigh = uint64_t(high) << 24;

				stamps.emplace_back(timestampHigh | timestampLow, header >> 8, (-1 * (seg[1] & 0x000000ff))/256);
				markers_buffer.advance_processed(16);

				break;
			}
			case 0x04:
			case 0x0a:
			{
				int block_total = 0;
				for (int i = 0; i < 4; i++)
				{
					int32_t *l_ptr = seg + (i * 4);
					if ((*l_ptr & 0x000000FF) == 0x04)
					{
						uint32_t s_lo = l_ptr[0];
						uint32_t s_hi = l_ptr[1];
						uint32_t e_lo = l_ptr[2];
						uint32_t e_hi = l_ptr[3];

						uint64_t start_block_index = (uint64_t(s_hi & 0xffffff) << 8) | ((s_lo >> 24) & 0xff);
						uint32_t start_block_sample_indx = (s_hi >> 24) & 0xff;
						uint64_t end_block_index = (uint64_t(e_hi & 0xffffff) << 8) | ((e_lo >> 24) & 0xff);
						uint32_t end_block_sample_indx = (e_hi >> 24) & 0xff;

						uint64_t to_acquire_memory_blocks_f = (end_block_index - start_block_index) * 4;

						if (end_block_index < start_block_index)
						{
							std::cout << "end_block_index < start_block_index\n";
							std::cout << "end_block_index: " << end_block_index << " start_block_index: " << start_block_index << "\n";
						
							// suggested in CPP_IVIC_StreamingZeroSuppress example project
							end_block_index += uint64_t(std::numeric_limits<unsigned int>::max());
						}

						if (to_acquire_memory_blocks_f % 16 != 0)
							to_acquire_memory_blocks_f = ((to_acquire_memory_blocks_f / 16) + 1) * 16;

						if (stamps.size() == 0)
						{
							std::cerr << "\tNo elements in stamps - discarding acquired elements." << std::endl;
							markers_buffer.advance_processed(16);
							break;
						}

						stamps.back().gate_data.emplace_back(
							start_block_index,
							start_block_sample_indx,
							end_block_index,
							end_block_sample_indx,
							to_acquire_memory_blocks_f);

						gate_count++;
						to_acquire += to_acquire_memory_blocks_f;
					}
					else if ((*l_ptr & 0x000000FF) == 0x0a)
					{
						// Do not use pre- or post-gate samples, so no need to store information at the moment
						uint32_t r_lo = l_ptr[0];
						uint32_t r_hi = l_ptr[1];

						uint64_t record_block_index = (uint64_t(r_hi & 0xffffff) << 8) | ((r_lo >> 24) & 0xff);
						uint32_t record_block_sample_indx = (r_hi >> 24) & 0xff;
					}
				}

				markers_buffer.advance_processed(16);
				break;
			}
			case 0x08:	// Should never get here
			{
				throw std::runtime_error("dummy gate error");
			}
			default:
				throw std::string("unexpected header -> (default) header: " + std::to_string(header & 0x000000FF));
			}
		}

		if (trig_count <= triggers_per_read)
		{
			markers_buffer.reset();

			int next_markers_to_acquire = markers_to_acquire;
	
			first_element_markers = 0;
			available_elements_markers = 0;
			actual_elements_markers = 0;

			do
			{
				markers_to_acquire = next_markers_to_acquire;

				auto rc = digitizer.stream_fetch_data(
						markers_channel.c_str(),
						markers_to_acquire,
						markers_buffer.get_size(),
						(ViInt32 *)markers_buffer.get_raw_unaquired(),
						&available_elements_markers, &actual_elements_markers, &first_element_markers);
				if (rc.second == Digitizer::None)
				{
					throw std::runtime_error(rc.first);
				}

				if (use_timeout && std::chrono::high_resolution_clock::now() > finish)
				{
					std::stringstream timeout_conditions;
					timeout_conditions <<  "timeout conditions:\n\n" \
					<< "markers_buffer.get_size(): " << std::to_string(markers_buffer.get_size()) << "\n" \
					<< "\tmarkers_buffer.get_acquired(): " << std::to_string(markers_buffer.get_acquired()) << "\n" \
					<< "\tmarkers_to_acquire: " << std::to_string(markers_to_acquire) << "\n" \
					<< "\tavailable_elements_markers: " << std::to_string(available_elements_markers) << "\n" \
					<< "\tactual_elements_markers: " << std::to_string(actual_elements_markers) << "\n\n" \
					<< "\ttrig_count: " << std::to_string(trig_count) << "\n";

					throw std::runtime_error("timeout in acquisition.\n" + timeout_conditions.str());
				}

			} while (actual_elements_markers == 0);

#ifdef variable_m_t_a
			if (available_elements_markers > markers_to_acquire && active_multiplier < multiplier_max)
			{
				markers_to_acquire = min_target_records * ++active_multiplier;
			}
#endif

			markers_buffer.advance_offset(first_element_markers);
			markers_buffer.advance_acquired(actual_elements_markers);
		}
	}

process:
	ViInt64 first_element_samples = 0;
	ViInt64 actual_elements_samples = 0;
	ViInt64 available_elements_samples = 0;

	do
	{
		if (to_acquire == 0)
			break;

		int actual_acquire = to_acquire - actual_elements_samples;
		auto rc = digitizer.stream_fetch_data(
				samples_channel.c_str(),
				actual_acquire,
				samples_buffer->get_size(),
				(ViInt32 *)samples_buffer->get_raw_unaquired(),
				&available_elements_samples, &actual_elements_samples, &first_element_samples);
		if (rc.second == Digitizer::None)
		{
			throw std::runtime_error(rc.first);
		}

		samples_buffer->advance_offset(first_element_samples);
		samples_buffer->advance_acquired(actual_elements_samples);
	} while (samples_buffer->get_acquired() < to_acquire);

	return AcquiredData(stamps, samples_buffer, samples_per_trigger);

}