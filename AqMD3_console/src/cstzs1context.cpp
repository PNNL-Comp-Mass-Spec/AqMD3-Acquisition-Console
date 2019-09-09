#include "../include/cstzs1context.h"

#include <iostream>

AcquiredData CstZm1Context::acquire(int32_t triggers, std::chrono::milliseconds timeoutMs)
{
	bool preprocess = false;
	AcquisitionBuffer* markersBuffer = nullptr;

	if (unprocessed_buf == nullptr)
	{
		//cout << "getting new buffer" << endl;
		markersBuffer = markers->next_available();
	}
	else
	{
		//cout << "getting unprocessed buffer" << endl;
		markersBuffer = unprocessed_buf;
		preprocess = true;
	}

	AcquisitionBuffer* samplesBuffer = samples->next_available();

	int min_target_records = triggers * 16;
	int trig_count = 0;
	uint64_t to_acquire = 0;

	ViInt64 firstElementMarkers;
	ViInt64 availableElementsMarkers = 0;
	ViInt64 actualElementsMarkers = unprocessed;
	vector<uint64_t> stamps;

	while (trig_count < triggers)
	{
		//std::cout << "trigs: " << trig_count << endl;
		//std::cout << "actualElementsMarkers: " << actualElementsMarkers << endl;
		for (int i = 0; i < actualElementsMarkers / 16; i++)
		{
			int32_t *seg = markersBuffer->get_raw_unprocessed();
			uint32_t header = seg[0];
			
			switch (header & 0x000000FF)
			{
			case 0x01:
			{
				if (trig_count >= triggers)
					goto process;

				uint64_t low = seg[1];
				uint64_t high = seg[2];
				uint64_t const timestampLow = (low >> 8) & 0x0000000000ffffffL;
				uint64_t const timestampHigh = uint64_t(high) << 24;

				stamps.push_back(timestampHigh | timestampLow);
				markersBuffer->advance_processed(16);
				trig_count++;

				break;
			}
			case 0x04:
			{
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

						size_t acq = (e - s) * 8;
						size_t ac = 16;
						to_acquire += (((acq + (ac - 1)) / ac) * ac);

						cout << "unaligned: " << (e - 1 - s - 1) * 8 << " | aligned: " << (((acq + (ac - 1)) / ac) * ac) << endl;

						//to_acquire += ((e - s) * 8);
					}
				}
				markersBuffer->advance_processed(16);
				break;
			}
			//std::cout << "ignore for now" << std::endl;
			//break;

			default:
				break;
			}
		}

		if (preprocess)
		{
			//cout << "preprocess = true" << endl;
			markers->return_in_use(markersBuffer);
			markersBuffer = markers->next_available();
			preprocess = false;
		}

		markersBuffer->reset_processed();
		//cout << "actual elems: " << actualElementsMarkers << " min targ elems: " << min_target_records << endl;
		//while (actualElementsMarkers <= min_target_records);
		if (trig_count < triggers)
		{
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
		}

		markersBuffer->advance_offset(firstElementMarkers);
		//markersBuffer->advance_acquired(actualElementsMarkers);

		//std::cout << "to acquire markers : " << min_target_records 
		//	<< " | actual elements : " << actualElementsMarkers 
		//	<< " | remaining elements: " << availableElementsMarkers 
		//	<< " | fist element indx: " << firstElementMarkers << endl;
	}

process:
	int left = markersBuffer->get_unprocessed();
	if (left > 0)
	{
		unprocessed = markersBuffer->get_unprocessed();
		unprocessed_buf = markersBuffer;
	}
	else
	{
		unprocessed = 0;
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
		(ViInt32 *)samplesBuffer->get_raw_data(),
		&availableElementsSamples, &actualElementsSamples, &firstElementSamples);

	samplesBuffer->advance_offset(firstElementSamples);
	samplesBuffer->advance_acquired(actualElementsSamples);

	//std::cout << "to acquire samples : " << to_acquire <<
	//	" | actual elements : " << actualElementsSamples <<
	//	" | remaining elements: " << availableElementsSamples <<
	//	" | first element index: " << firstElementSamples << endl;

	return AcquiredData(stamps, samples, samplesBuffer);
}
