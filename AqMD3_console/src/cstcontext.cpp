#include "../include/cstcontext.h"
#include "../include/acquisitionbuffer.h"

#include <iostream>

AcquiredData CstContext::acquire(std::chrono::milliseconds timeoutMs)
{
	int markers_to_gather = triggers_per_read * 16;
	vector<int32_t> markers(markers_to_gather * 16);
	AcquisitionBuffer* samples_buffer = samples_buffer_pool->next_available();

	ViInt64 first_element_offset;
	ViInt64 available_elements_markers = 0;
	ViInt64 actual_elements_markers = 0;

	do
	{
		AqMD3_StreamFetchDataInt32(
			session,
			markers_channel.c_str(),
			markers_to_gather,
			markers.size(),
			(ViInt32 *)markers.data(),
			&available_elements_markers, &actual_elements_markers, &first_element_offset);
	} while (actual_elements_markers < markers_to_gather);

	vector<TriggerData> stamps;

	int32_t *ptr = markers.data() + first_element_offset;
	int32_t offset = 0;
	for (int i = 0; i < triggers_per_read; i++)
	{
		int32_t *seg = ptr + offset;
		uint32_t header = seg[0];

		if ((header & 0x000000FF) != 0x01)
			cerr << "wrong header -- cst acq (not zero sp)\n";

		uint64_t low = seg[1];
		uint64_t high = seg[2];
		uint64_t timestampLow = (low >> 8) & 0x0000000000ffffffL;
		uint64_t timestampHigh = uint64_t(high) << 24;
		std::cout << "\tindex: " << (header >> 8) << endl;
		stamps.emplace_back(timestampHigh | timestampLow, header >> 8, (-1 * (seg[1] & 0x000000ff)) / 256);

		offset += 16;
	}

	ViInt64 firstElementSamples;
	ViInt64 actualElementsSamples = 0;
	ViInt64 availableElementsSamples = 0;

	int elements_to_gather = (triggers_per_read * samples_per_trigger) / 2;
	
	do
	{
		AqMD3_StreamFetchDataInt32(
			session,
			samples_channel.c_str(),
			elements_to_gather,
			samples_buffer->get_size(),
			(ViInt32 *)samples_buffer->get_raw_data(),
			&availableElementsSamples, &actualElementsSamples, &firstElementSamples);
	} while (actualElementsSamples < elements_to_gather);
	//std::cout << "availableElementsSamples " << availableElementsSamples << std::endl
	//	<< "actualElementsSamples: " << actualElementsSamples << std::endl
	//	<< "firstElementSamples: " << firstElementSamples << std::endl;


	samples_buffer->advance_offset(firstElementSamples);
	samples_buffer->advance_acquired(actualElementsSamples);

	return AcquiredData(stamps, samples_buffer_pool, samples_buffer, samples_per_trigger);
}
