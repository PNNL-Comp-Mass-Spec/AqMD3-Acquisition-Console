#include "../include/cstcontext.h"
#include "../include/acquisitionbuffer.h"

#include <iostream>

//AcquiredData CstContext::acquire(int32_t triggers, std::chrono::milliseconds timeoutMs)
//{
//	//std::cout << "getting buffers" << std::endl;
//
//	AcquisitionBuffer* markersBuffer = markers->next_available();
//	//std::cout << "got markers buffer" << std::endl;
//
//	AcquisitionBuffer* samplesBuffer = samples->next_available();
//	//std::cout << "got samples buffer" << std::endl;
//
//	ViInt64 firstElementMarkers;
//	ViInt64 availableElementsMarkers = 0;
//	ViInt64 actualElementsMarkers;
//
//	int markersToGather = triggers * 16;
//	do
//	{
//		AqMD3_StreamFetchDataInt32(
//			session,
//			markersChannel.c_str(),
//			markersToGather,
//			markersBuffer->get_size(),
//			(ViInt32 *)markersBuffer->get_raw_data(),
//			&availableElementsMarkers, &actualElementsMarkers, &firstElementMarkers);
//	} while (actualElementsMarkers < markersToGather);
//
//	//std::cout << "availableElementsMarkers: " << availableElementsMarkers << std::endl
//	//	<< "actualElementsMarkers: " << actualElementsMarkers << std::endl
//	//	<< "firstElementMarkers: " << firstElementMarkers << std::endl;
//
//	markersBuffer->advance_offset(firstElementMarkers);
//	markersBuffer->advance_acquired(actualElementsMarkers);
//
//	ViInt64 firstElementSamples;
//	ViInt64 actualElementsSamples = 0;
//	ViInt64 availableElementsSamples;
//
//	int samplesToGather = (triggers * 94016) / 2;
//	//std::cout << "samples to gather " << samplesToGather << std::endl;
//	
//	do
//	{
//		AqMD3_StreamFetchDataInt32(
//			session,
//			samplesChannel.c_str(),
//			samplesToGather,
//			samplesBuffer->get_size(),
//			(ViInt32 *)samplesBuffer->get_raw_data(),
//			&availableElementsSamples, &actualElementsSamples, &firstElementSamples);
//	} while (actualElementsSamples < samplesToGather);
//	//std::cout << "availableElementsSamples " << availableElementsSamples << std::endl
//	//	<< "actualElementsSamples: " << actualElementsSamples << std::endl
//	//	<< "firstElementSamples: " << firstElementSamples << std::endl;
//
//
//	samplesBuffer->advance_offset(firstElementSamples);
//	samplesBuffer->advance_acquired(actualElementsSamples);
//
//	return AcquiredData(samples, markers, markersBuffer, samplesBuffer);
//}
