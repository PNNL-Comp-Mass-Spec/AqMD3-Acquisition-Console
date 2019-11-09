#include "../include/cstzs1context.h"

#include <vector>
#include <tuple>
#include <exception>

using namespace std;

#define bad_gates

AcquiredData CstZm1Context::acquire(std::chrono::milliseconds timeoutMs)
{
	int markers_to_acquire = min_target_records;
	int trig_count = 0;
	int gate_count = 0;
	uint64_t to_acquire = 0;
	vector<AcquiredData::TriggerData> stamps;
	shared_ptr<AcquisitionBuffer> samples_buffer = get_buffer();

	ViInt64 first_element_markers;
	ViInt64 available_elements_markers = 0;
	ViInt64 actual_elements_markers = markers_buffer.get_unprocessed();

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

#ifdef bad_gates
						if (to_acquire_memory_blocks_f == 4)
						{
							cerr << "gate block difference == 1, (stop samples indx - start samples indx) < 1" << endl;
							continue;
						}
#endif

						if (to_acquire_memory_blocks_f % 16 != 0)
							to_acquire_memory_blocks_f = ((to_acquire_memory_blocks_f / 16) + 1) * 16;

						if (stamps.size() == 0)
						{
							cerr << "\tNo elements in stamps - discarding acquired elements." << endl;
							markers_buffer.advance_processed(16);
							break;
						}

						stamps.back().gate_data.emplace_back(
							start_block_index,
							start_block_sample_indx,
							end_block_index,
							end_block_sample_indx,
							to_acquire_memory_blocks_f);

						++gate_count;

						to_acquire += to_acquire_memory_blocks_f;
					}
					else if ((*l_ptr & 0x000000FF) == 0x0a)
					{
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
				cerr << "\tERROR -- DUMMY GATE\n";
				throw string("dummy gate error");
			}
			default:
				cerr << "\t(default) header: " << (header & 0x000000FF) << endl;
				throw string("unexpected header");
				break;
			}
		}

		first_element_markers = 0;
		available_elements_markers = 0;
		actual_elements_markers = 0;
		if (trig_count <= triggers_per_read)
		{
			markers_buffer.reset();
			do
			{
				AqMD3_StreamFetchDataInt32(
					session,
					markers_channel.c_str(),
					markers_to_acquire,
					markers_buffer.get_size(),
					(ViInt32 *)markers_buffer.get_raw_unaquired(),
					&available_elements_markers, &actual_elements_markers, &first_element_markers);

				if (std::chrono::high_resolution_clock::now() > finish)
					goto process;

			} while (actual_elements_markers < markers_to_acquire);

			if (available_elements_markers > markers_to_acquire)
				markers_to_acquire *= gate_acquisition_multiplier;

			markers_buffer.advance_offset(first_element_markers);
			markers_buffer.advance_acquired(actual_elements_markers);
		}
	}

process:
	ViInt64 first_element_samples;
	ViInt64 actual_elements_samples = 0;
	ViInt64 available_elements_samples = 0;
	AqMD3_StreamFetchDataInt32(
		session,
		samples_channel.c_str(),
		to_acquire,
		samples_buffer->get_size(),
		(ViInt32 *)samples_buffer->get_raw_unaquired(),
		&available_elements_samples, &actual_elements_samples, &first_element_samples);

	//cout << "to_acquire: " << to_acquire << endl
	//	<< "available_elements_samples: " << available_elements_samples << endl
	//	<< "actual_elements_samples: " << actual_elements_samples << endl;

	samples_buffer->advance_offset(first_element_samples);
	samples_buffer->advance_acquired(actual_elements_samples);

	return AcquiredData(stamps, samples_buffer, samples_per_trigger);
}