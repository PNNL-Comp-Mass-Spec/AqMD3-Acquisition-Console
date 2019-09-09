#include "../include/digitizer.h"
#include "../include/cstcontext.h"
#include "../include/cstzs1context.h"
#include "../include/dgtcontext.h"

#include <iostream>

//std::unique_ptr<StreamingContext> Digitizer::configure_cst()
//{
//	// Acquisition configuration parameters
//	ViReal64 const sampleRate = 1.0e9;
//	ViReal64 const sampleInterval = 1.0 / sampleRate;
//	ViInt64 const recordSize = 94016;
//	ViInt32 const streamingMode = AQMD3_VAL_STREAMING_MODE_TRIGGERED;
//	ViInt32 const acquisitionMode = AQMD3_VAL_ACQUISITION_MODE_NORMAL;
//
//	ViConstString channel = "Channel1";
//	ViReal64 const range = 2.5;
//	ViReal64 const offset = 0.0;
//	ViInt32 const coupling = AQMD3_VAL_VERTICAL_COUPLING_DC;
//
//	// Trigger configuration
//	ViConstString triggerSource = "External1";
//	ViReal64 const triggerLevel = 0.50;
//	ViInt32 const triggerSlope = AQMD3_VAL_TRIGGER_SLOPE_POSITIVE;
//
//	ViConstString sampleStreamName = "StreamCh1";
//	ViConstString markerStreamName = "MarkersCh1";
//	ViInt64 const nbrOfRecordsToFetchAtOnce = 256;
//
//	AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_STREAMING_MODE, streamingMode);
//	AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate);
//	AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_ACQUISITION_MODE, acquisitionMode);
//	AqMD3_SetAttributeViInt64(session, "", AQMD3_ATTR_RECORD_SIZE, recordSize);
//	AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_ACQUISITION_DATA_REDUCTION_MODE, AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_DISABLED);
//
//
//	AqMD3_ConfigureChannel(session, channel, range, offset, coupling, VI_TRUE);
//
//	AqMD3_SetAttributeViString(session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, triggerSource);
//	AqMD3_SetAttributeViReal64(session, triggerSource, AQMD3_ATTR_TRIGGER_LEVEL, triggerLevel);
//	AqMD3_SetAttributeViInt32(session, triggerSource, AQMD3_ATTR_TRIGGER_SLOPE, triggerSlope);
//
//	AqMD3_ApplySetup(session);
//	AqMD3_SelfCalibrate(session);
//
//	ViInt64 sampleStreamGrain = 0;
//	ViInt64 markerStreamGrain = 0;
//	AqMD3_GetAttributeViInt64(session, sampleStreamName, AQMD3_ATTR_STREAM_GRANULARITY_IN_BYTES, &sampleStreamGrain);
//	AqMD3_GetAttributeViInt64(session, markerStreamName, AQMD3_ATTR_STREAM_GRANULARITY_IN_BYTES, &markerStreamGrain);
//	ViInt64 const sampleStreamGrainElements = sampleStreamGrain / sizeof(int32_t);
//	ViInt64 const markerStreamGrainElements = markerStreamGrain / sizeof(int32_t);
//
//	ViInt64 const nbrSamplesPerElement = 2;
//
//	ViInt64 const recordElements = recordSize / nbrSamplesPerElement;
//	ViInt64 const acquisitionElements = recordElements * nbrOfRecordsToFetchAtOnce;
//	ViInt64 const markerElements = 16 * nbrOfRecordsToFetchAtOnce;
//
//	ViInt64 const sampleStreamBufferSize = acquisitionElements	// required elements
//		+ acquisitionElements / 2								// unwrapping overhead (only in single channel mode)
//		+ sampleStreamGrainElements - 1;						// alignment overhead
//
//	ViInt64 const markerStreamBufferSize = markerElements	// required elements
//		+ markerStreamGrainElements - 1;					// alignment overhead
//
//	AcquisitionBufferPool *data = new AcquisitionBufferPool(sampleStreamBufferSize, 8);
//	AcquisitionBufferPool *markers = new AcquisitionBufferPool(markerStreamBufferSize, 8);
//
//	//std::cout << data.cou
//
//	std::unique_ptr<CstContext> context(new CstContext(session, data, markers, markerStreamName, sampleStreamName));
//
//	return std::move(context);
//}

std::shared_ptr<StreamingContext> Digitizer::configure_cst_zs1()
{	
	ViReal64 const sampleRate = 1.0e9;
	ViReal64 const sampleInterval = 1.0 / sampleRate;
	ViInt64 const recordSize = 94016;
	ViInt32 const streamingMode = AQMD3_VAL_STREAMING_MODE_TRIGGERED;
	ViInt32 const dataReductionMode = AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_ZERO_SUPPRESS;
	
	ViConstString channel = "Channel1";
	ViReal64 const range = 2.5;
	ViReal64 const offset = 0.0;
	ViInt32 const coupling = AQMD3_VAL_VERTICAL_COUPLING_DC;
	
	//ViInt32 const zsThreshold = 4200;
	//ViInt32 const zsThreshold = 2000;
	ViInt32 const zsThreshold = 0;
	ViInt32 const zsHysteresis = 300;
	ViInt32 const zsPreGateSamples = 0;
	ViInt32 const zsPostGateSamples = 0;
	
	ViConstString triggerSource = "External1";
	ViReal64 const triggerLevel = 0.50;
	ViInt32 const triggerSlope = AQMD3_VAL_TRIGGER_SLOPE_POSITIVE;
	
	ViInt64 const nbrSampleElementsToFetch = 94016 * 256;					// enough to cover
	ViInt64 const nbrMarkerElementsToFetch = 16 * 256 * 256;				// all cases
	ViInt64 const nbrSampleBufferElements = nbrSampleElementsToFetch * 4;	// and then add
	ViInt64 const nbrMarkerBufferElements = nbrMarkerElementsToFetch * 4;	// even more
	
	AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_STREAMING_MODE, streamingMode);
	AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate);
	AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_ACQUISITION_DATA_REDUCTION_MODE, dataReductionMode);
	AqMD3_SetAttributeViInt64(session, "", AQMD3_ATTR_RECORD_SIZE, recordSize);
	
	AqMD3_ConfigureChannel(session, channel, range, offset, coupling, VI_TRUE);
	
	AqMD3_SetAttributeViInt32(session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_HYSTERESIS, zsHysteresis);
	AqMD3_SetAttributeViInt32(session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_THRESHOLD, zsThreshold);
	AqMD3_SetAttributeViInt32(session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_PRE_GATE_SAMPLES, zsPreGateSamples);
	AqMD3_SetAttributeViInt32(session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_POST_GATE_SAMPLES, zsPostGateSamples);
	
	AqMD3_SetAttributeViString(session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, triggerSource);
	AqMD3_SetAttributeViReal64(session, triggerSource, AQMD3_ATTR_TRIGGER_LEVEL, triggerLevel);
	AqMD3_SetAttributeViInt32(session, triggerSource, AQMD3_ATTR_TRIGGER_SLOPE, triggerSlope);

	ViInt64 sampleStreamGrain = 0;
	ViInt64 markerStreamGrain = 0;
	AqMD3_GetAttributeViInt64(session, "StreamCh1", AQMD3_ATTR_STREAM_GRANULARITY_IN_BYTES, &sampleStreamGrain);
	AqMD3_GetAttributeViInt64(session, "MarkersCh1", AQMD3_ATTR_STREAM_GRANULARITY_IN_BYTES, &markerStreamGrain);

	AqMD3_ApplySetup(session);
	AqMD3_SelfCalibrate(session);

	AcquisitionBufferPool *data = new AcquisitionBufferPool(nbrSampleBufferElements, 8);
	AcquisitionBufferPool *markers = new AcquisitionBufferPool(nbrMarkerBufferElements, 8);

	std::shared_ptr<CstZm1Context> context(new CstZm1Context(session, data, markers, "MarkersCh1","StreamCh1", markerStreamGrain, sampleStreamGrain));
	return std::move(context);
}

//std::unique_ptr<AcquisitionContext> Digitizer::configure_dgt()
//{
//	ViInt64 const recordSize = 94016;
//	ViReal64 const range = 2.5;
//	ViReal64 const offset = 0.0;
//
//	ViReal64 const sampleRate = 1.0e9;
//
//	ViInt32 const coupling = AQMD3_VAL_VERTICAL_COUPLING_DC;
//	AqMD3_ConfigureChannel(session, "Channel1", range, offset, coupling, VI_TRUE);
//	
//	//
//	AqMD3_SetAttributeViInt64(session, "", AQMD3_ATTR_NUM_RECORDS_TO_ACQUIRE, 200);
//	//
//
//	AqMD3_SetAttributeViInt64(session, "", AQMD3_ATTR_RECORD_SIZE, recordSize);
//	AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate);
//
//	AqMD3_SetAttributeViString(session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, "External1");
//	AqMD3_SetAttributeViReal64(session, "External1", AQMD3_ATTR_TRIGGER_LEVEL, 0.5);
//	AqMD3_SetAttributeViInt32(session, "External1", AQMD3_ATTR_TRIGGER_SLOPE, AQMD3_VAL_TRIGGER_SLOPE_POSITIVE);
//
//	AqMD3_SelfCalibrate(session);
//
//	std::unique_ptr<DgtContext> ctx(new DgtContext(session, "Channel1"));
//
//	return std::move(ctx);
//}
