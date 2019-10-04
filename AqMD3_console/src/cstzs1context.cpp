#include "../include/cstzs1context.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <exception>

using namespace std;

AcquiredData CstZm1Context::acquire(std::chrono::milliseconds timeoutMs)
{
	const int gate_acquisition_multiplier = 2;
	const int marker_hunk_size = 16;
	const int max_targer_records = triggers_per_read * marker_hunk_size * gate_acquisition_multiplier;
	const int min_target_records = triggers_per_read * marker_hunk_size;

	int markers_to_acquire = min_target_records;
	int trig_count = 0;
	int gate_count = 0;
	uint64_t to_acquire = 0;
	vector<AcquiredData::TriggerData> stamps;

	bool preprocess = false;

	AcquisitionBuffer* markers_buffer = nullptr;
	if (unprocessed_buffer == nullptr)
	{
		markers_buffer = markers_buffer_pool.next_available();
	}
	else
	{
		markers_buffer = unprocessed_buffer;
		preprocess = true;
	}

	AcquisitionBuffer* samples_buffer = samples_buffer_pool->next_available();

	ViInt64 first_element_markers;
	ViInt64 available_elements_markers = 0;
	ViInt64 actual_elements_markers = markers_buffer->get_unprocessed();
	while (trig_count < triggers_per_read)
	{
		for (int i = 0; i < actual_elements_markers / 16; i++)
		{
			int32_t *seg = markers_buffer->get_raw_unprocessed();
			uint32_t header = seg[0];

			switch (header & 0x000000FF)
			{
			case 0x01:
			{
				++trig_count;
				if (trig_count >= triggers_per_read)
					goto process;

				uint64_t low = seg[1];
				uint64_t high = seg[2];
				uint64_t timestampLow = (low >> 8) & 0x0000000000ffffffL;
				uint64_t timestampHigh = uint64_t(high) << 24;
				//std::cout << "\tindex: " << (header >> 8) << endl;
				stamps.emplace_back(timestampHigh | timestampLow, header >> 8, (-1 * (seg[1] & 0x000000ff))/256);
				markers_buffer->advance_processed(16);

				break;
			}
			case 0x04:
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

						uint64_t s = (uint64_t(s_hi & 0xffffff) << 8) | ((s_lo >> 24) & 0xff);
						uint64_t e = (uint64_t(e_hi & 0xffffff) << 8) | ((e_lo >> 24) & 0xff);

						uint64_t to_acquire_samples = (e - s) * 8;
						uint64_t to_acquire_memory_blocks = to_acquire_samples / 2;

						while (to_acquire_memory_blocks % 16 != 0)
							to_acquire_memory_blocks += 4;

						if (stamps.size() == 0)
						{
							cout << "\tNo elements in stamps - discarding acquired elements." << endl;
							markers_buffer->advance_processed(16);
							break;
						}

						stamps.back().gate_data.emplace_back((s - 1) * 8, (e - 1) * 8, to_acquire_samples, to_acquire_memory_blocks);
						++gate_count;

						to_acquire += to_acquire_memory_blocks;
					}
				}

				markers_buffer->advance_processed(16);
				break;
			}
			case 0x08:	// Should never get here
			{
				cout << "\tERROR -- DUMMY GATE\n";
				exception("dummy gate error");
			}
			default:
				break;
			}
		}

		if (preprocess)
		{
			markers_buffer_pool.return_in_use(markers_buffer);
			markers_buffer = markers_buffer_pool.next_available();
			unprocessed_buffer = nullptr;
			preprocess = false;
		}

		first_element_markers = 0;
		available_elements_markers = 0;
		actual_elements_markers = 0;
		if (trig_count < triggers_per_read)
		{
			markers_buffer->reset();
			do
			{
				AqMD3_StreamFetchDataInt32(
					session,
					markers_channel.c_str(),
					markers_to_acquire,
					markers_buffer->get_size(),
					(ViInt32 *)markers_buffer->get_raw_unaquired(),
					&available_elements_markers, &actual_elements_markers, &first_element_markers);

			} while (actual_elements_markers < markers_to_acquire);

			if (available_elements_markers > markers_to_acquire)
				markers_to_acquire *= gate_acquisition_multiplier;

			//cout << "\tacquired marker elements: " << actual_elements_markers 
			//	<< " -- available marker elements: " << available_elements_markers
			//	<<  " -- first element index: " << first_element_markers << endl;

			markers_buffer->advance_offset(first_element_markers);
			markers_buffer->advance_acquired(actual_elements_markers);
		}
	}

process:
	if (markers_buffer->get_unprocessed() > 0)
	{
		unprocessed_buffer = markers_buffer;
	}
	else
	{
		unprocessed_buffer = nullptr;
		markers_buffer_pool.return_in_use(markers_buffer);
	}

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

	samples_buffer->advance_offset(first_element_samples);
	samples_buffer->advance_acquired(actual_elements_samples);

	//cout << "\tacquired sample elements: " << actual_elements_samples << endl;

	return AcquiredData(stamps, samples_buffer_pool, samples_buffer, samples_per_trigger);
}