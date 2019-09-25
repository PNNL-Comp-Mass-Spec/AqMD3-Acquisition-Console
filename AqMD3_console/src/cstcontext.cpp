#include "../include/cstcontext.h"
#include "../include/acquisitionbuffer.h"

#include <iostream>

AcquiredData CstContext::acquire(int32_t triggers, std::chrono::milliseconds timeoutMs)
{
	AcquisitionBuffer* markersBuffer = markers->next_available();
	AcquisitionBuffer* samplesBuffer = samples->next_available();

	ViInt64 firstElementMarkers;
	ViInt64 availableElementsMarkers = 0;
	ViInt64 actualElementsMarkers = 0;

	int markersToGather = triggers * 16;
	do
	{
		AqMD3_StreamFetchDataInt32(
			session,
			markersChannel.c_str(),
			markersToGather,
			markersBuffer->get_size(),
			(ViInt32 *)markersBuffer->get_raw_data(),
			&availableElementsMarkers, &actualElementsMarkers, &firstElementMarkers);
	} while (actualElementsMarkers < markersToGather);

	//std::cout << "availableElementsMarkers: " << availableElementsMarkers << std::endl
	//	<< "actualElementsMarkers: " << actualElementsMarkers << std::endsl
	//	<< "firstElementMarkers: " << firstElementMarkers << std::endl;

	markersBuffer->advance_offset(firstElementMarkers);
	markersBuffer->advance_acquired(actualElementsMarkers);

	vector<uint64_t> stamps;

	for (int i = 0; i < triggers; i++)
	{
		int32_t *seg = markersBuffer->get_raw_unprocessed();
		uint32_t header = seg[0];

		uint64_t low = seg[1];
		uint64_t high = seg[2];
		uint64_t const timestampLow = (low >> 8) & 0x0000000000ffffffL;
		uint64_t const timestampHigh = uint64_t(high) << 24;

		stamps.push_back(timestampHigh | timestampLow);
		markersBuffer->advance_processed(16);
	}

	ViInt64 firstElementSamples;
	ViInt64 actualElementsSamples = 0;
	ViInt64 availableElementsSamples = 0;

	int samplesToGather = (triggers * samples_per_trigger) / 2;
	
	do
	{
		AqMD3_StreamFetchDataInt32(
			session,
			samplesChannel.c_str(),
			samplesToGather,
			samplesBuffer->get_size(),
			(ViInt32 *)samplesBuffer->get_raw_data(),
			&availableElementsSamples, &actualElementsSamples, &firstElementSamples);
	} while (actualElementsSamples < samplesToGather);
	//std::cout << "availableElementsSamples " << availableElementsSamples << std::endl
	//	<< "actualElementsSamples: " << actualElementsSamples << std::endl
	//	<< "firstElementSamples: " << firstElementSamples << std::endl;


	samplesBuffer->advance_offset(firstElementSamples);
	samplesBuffer->advance_acquired(actualElementsSamples);

	//return AcquiredData(stamps, samples, samplesBuffer);
	return AcquiredData();
}
