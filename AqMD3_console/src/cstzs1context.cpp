#include "../include/cstzs1context.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <exception>

AcquiredData CstZm1Context::acquire(int32_t triggers, std::chrono::milliseconds timeoutMs)
{
	int trig_count = 0;
	int gate_count = 0;
	uint64_t to_acquire = 0;
	vector<TriggerData> stamps;
	bool preprocess = false;
	int min_target_records = triggers * 16;

	AcquisitionBuffer* markersBuffer = nullptr;
	if (unprocessed_buf == nullptr)
	{
		markersBuffer = markers->next_available();
	}
	else
	{
		markersBuffer = unprocessed_buf;
		preprocess = true;
	}

	AcquisitionBuffer* samplesBuffer = samples->next_available();

	ViInt64 firstElementMarkers;
	ViInt64 availableElementsMarkers = 0;
	ViInt64 actualElementsMarkers = markersBuffer->get_unprocessed();

	while (trig_count < triggers)
	{
		for (int i = 0; i < actualElementsMarkers / 16; i++)
		{
			int32_t *seg = markersBuffer->get_raw_unprocessed();
			uint32_t header = seg[0];

			switch (header & 0x000000FF)
			{
			case 0x01:
			{
				++trig_count;
				if (trig_count > triggers)
					goto process;

				uint64_t low = seg[1];
				uint64_t high = seg[2];
				uint64_t timestampLow = (low >> 8) & 0x0000000000ffffffL;
				uint64_t timestampHigh = uint64_t(high) << 24;
				std::cout << "\tindex: " << (header >> 8) << endl;
				stamps.emplace_back(timestampHigh | timestampLow, header >> 8, (-1 * (seg[1] & 0x000000ff))/256);
				markersBuffer->advance_processed(16);

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
							markersBuffer->advance_processed(16);
							break;
						}

						stamps.back().gate_cage.emplace_back((s - 1) * 8, (e - 1) * 8, to_acquire_samples, to_acquire_memory_blocks);
						++gate_count;

						to_acquire += to_acquire_memory_blocks;
					}
				}

				markersBuffer->advance_processed(16);
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
			markers->return_in_use(markersBuffer);
			markersBuffer = markers->next_available();
			preprocess = false;
		}

		firstElementMarkers = 0;
		availableElementsMarkers = 0;
		actualElementsMarkers = 0;
		if (trig_count < triggers)
		{
			markersBuffer->reset();
			do
			{
				AqMD3_StreamFetchDataInt32(
					session,
					markersChannel.c_str(),
					min_target_records,
					markersBuffer->get_size(),
					(ViInt32 *)markersBuffer->get_raw_unaquired(),
					&availableElementsMarkers, &actualElementsMarkers, &firstElementMarkers);
			} while (actualElementsMarkers < min_target_records);

			cout << "\tacquired: " << actualElementsMarkers << endl;

			markersBuffer->advance_offset(firstElementMarkers);
			markersBuffer->advance_acquired(actualElementsMarkers);
		}
	}

process:
	if (markersBuffer->get_unprocessed() > 0)
	{
		unprocessed_buf = markersBuffer;
	}
	else
	{
		unprocessed_buf = nullptr;
		markers->return_in_use(markersBuffer);
	}

	ViInt64 firstElementSamples;
	ViInt64 actualElementsSamples = 0;
	ViInt64 availableElementsSamples = 0;
	AqMD3_StreamFetchDataInt32(
		session,
		samplesChannel.c_str(),
		to_acquire,
		samplesBuffer->get_size(),
		(ViInt32 *)samplesBuffer->get_raw_unaquired(),
		&availableElementsSamples, &actualElementsSamples, &firstElementSamples);

	samplesBuffer->advance_offset(firstElementSamples);
	samplesBuffer->advance_acquired(actualElementsSamples);

	return AcquiredData(stamps, samples, samplesBuffer);
}