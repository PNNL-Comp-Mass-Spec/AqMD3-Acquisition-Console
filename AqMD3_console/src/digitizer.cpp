#include "../include/digitizer.h"
#include "../include/cstcontext.h"
#include "../include/cstzs1context.h"

/* non-permanent function not intended to be part of the final interface */
std::unique_ptr<StreamingContext> Digitizer::configure_cst()
{
	ViSession *session = nullptr;

	// Acquisition configuration parameters
	ViReal64 const sampleRate = 1.0e9;
	ViReal64 const sampleInterval = 1.0 / sampleRate;
	ViInt64 const recordSize = 94016;
	ViInt32 const streamingMode = AQMD3_VAL_STREAMING_MODE_TRIGGERED;
	ViInt32 const acquisitionMode = AQMD3_VAL_ACQUISITION_MODE_NORMAL;

	ViConstString channel = "Channel1";
	ViReal64 const range = 2.5;
	ViReal64 const offset = 0.0;
	ViInt32 const coupling = AQMD3_VAL_VERTICAL_COUPLING_DC;

	ViInt32 const zsThreshold = 0.05;
	ViInt32 const zsHysteresis = 300;
	ViInt32 const zsPreGateSamples = 0;
	ViInt32 const zsPostGateSamples = 0;

	// Trigger configuration
	ViConstString triggerSource = "External1";
	ViReal64 const triggerLevel = 0.10;
	ViInt32 const triggerSlope = AQMD3_VAL_TRIGGER_SLOPE_POSITIVE;

	ViConstString sampleStreamName = "StreamCh1";
	ViConstString markerStreamName = "MarkersCh1";
	ViInt64 const nbrOfRecordsToFetchAtOnce = 256;

	AqMD3_InitWithOptions((ViChar *)digitizer.c_str(), VI_FALSE, VI_FALSE, options.c_str(), session);

	AqMD3_SetAttributeViInt32(*session, "", AQMD3_ATTR_STREAMING_MODE, streamingMode);
	AqMD3_SetAttributeViReal64(*session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate);
	AqMD3_SetAttributeViInt32(*session, "", AQMD3_ATTR_ACQUISITION_MODE, acquisitionMode);
	AqMD3_SetAttributeViInt64(*session, "", AQMD3_ATTR_RECORD_SIZE, recordSize);

	AqMD3_ConfigureChannel(*session, channel, range, offset, coupling, VI_TRUE);

	AqMD3_SetAttributeViInt32(*session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_HYSTERESIS, zsHysteresis);
	AqMD3_SetAttributeViInt32(*session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_THRESHOLD, zsThreshold);
	AqMD3_SetAttributeViInt32(*session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_PRE_GATE_SAMPLES, zsPreGateSamples);
	AqMD3_SetAttributeViInt32(*session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_POST_GATE_SAMPLES, zsPostGateSamples);

	AqMD3_SetAttributeViString(*session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, triggerSource);
	AqMD3_SetAttributeViReal64(*session, triggerSource, AQMD3_ATTR_TRIGGER_LEVEL, triggerLevel);
	AqMD3_SetAttributeViInt32(*session, triggerSource, AQMD3_ATTR_TRIGGER_SLOPE, triggerSlope);

	AqMD3_ApplySetup(*session);
	AqMD3_SelfCalibrate(*session);

	ViInt64 sampleStreamGrain = 0;
	ViInt64 markerStreamGrain = 0;
	AqMD3_GetAttributeViInt64(*session, sampleStreamName, AQMD3_ATTR_STREAM_GRANULARITY_IN_BYTES, &sampleStreamGrain);
	AqMD3_GetAttributeViInt64(*session, markerStreamName, AQMD3_ATTR_STREAM_GRANULARITY_IN_BYTES, &markerStreamGrain);
	ViInt64 const sampleStreamGrainElements = sampleStreamGrain / sizeof(int32_t);
	ViInt64 const markerStreamGrainElements = markerStreamGrain / sizeof(int32_t);

	ViInt64 const nbrSamplesPerElement = (acquisitionMode == AQMD3_VAL_ACQUISITION_MODE_AVERAGER) ? 1
		: (acquisitionMode == AQMD3_VAL_ACQUISITION_MODE_NORMAL) ? 2
		: throw std::logic_error("Unexpected acquisition mode");

	ViInt64 const recordElements = recordSize / nbrSamplesPerElement;
	ViInt64 const acquisitionElements = recordElements * nbrOfRecordsToFetchAtOnce;
	ViInt64 const markerElements = 16 * nbrOfRecordsToFetchAtOnce;

	ViInt64 const sampleStreamBufferSize = acquisitionElements	// required elements
		+ acquisitionElements / 2								// unwrapping overhead (only in single channel mode)
		+ sampleStreamGrainElements - 1;						// alignment overhead

	ViInt64 const markerStreamBufferSize = markerElements	// required elements
		+ markerStreamGrainElements - 1;					// alignment overhead

	AcquisitionBufferPool sampleStreamBuffer(sampleStreamBufferSize, 32);
	AcquisitionBufferPool markerStreamBuffer(markerStreamBufferSize, 32);

	AcquisitionBufferPool data(sampleStreamBufferSize, 32);
	AcquisitionBufferPool markers(markerStreamBufferSize, 32);

	std::unique_ptr<CstContext> context(new CstContext(session, data, markers, markerStreamName, sampleStreamName));

	return std::move(context);
}

/* non-permanent function not intended to be part of the final interface */
std::unique_ptr<StreamingContext> Digitizer::configure_cst_zs1()
{
	//ViSession *session = nullptr;
	//
	//ViReal64 const sampleRate = 1.0e9;
	//ViReal64 const sampleInterval = 1.0 / sampleRate;
	//ViInt64 const recordSize = 1024;
	//ViInt32 const streamingMode = AQMD3_VAL_STREAMING_MODE_TRIGGERED;
	//ViInt32 const dataReductionMode = AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_ZERO_SUPPRESS;
	//
	//ViConstString channel = "Channel1";
	//ViReal64 const range = 2.5;
	//ViReal64 const offset = 0.0;
	//ViInt32 const coupling = AQMD3_VAL_VERTICAL_COUPLING_DC;
	//
	//ViInt32 const zsThreshold = 0.05;
	//ViInt32 const zsHysteresis = 300;
	//ViInt32 const zsPreGateSamples = 0;
	//ViInt32 const zsPostGateSamples = 0;
	//
	//// Trigger configuration
	//ViConstString triggerSource = "External1";
	//ViReal64 const triggerLevel = 0.10;
	//ViInt32 const triggerSlope = AQMD3_VAL_TRIGGER_SLOPE_POSITIVE;
	//
	//ViInt64 const nbrSampleElementsToFetch = 3121 * 32 * 32;
	//ViInt64 const nbrMarkerElementsToFetch = 64 * 1024;
	//ViInt64 const nbrSampleBufferElements = nbrSampleElementsToFetch * 2;
	//ViInt64 const nbrMarkerBufferElements = nbrMarkerElementsToFetch * 2;
	//
	//AqMD3_InitWithOptions((ViChar *)digitizer.c_str(), VI_FALSE, VI_FALSE, options.c_str(), session);
	//
	//AqMD3_SetAttributeViInt32(*session, "", AQMD3_ATTR_STREAMING_MODE, streamingMode);
	//AqMD3_SetAttributeViReal64(*session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate);
	//AqMD3_SetAttributeViInt32(*session, "", AQMD3_ATTR_ACQUISITION_DATA_REDUCTION_MODE, dataReductionMode);
	//AqMD3_SetAttributeViInt64(*session, "", AQMD3_ATTR_RECORD_SIZE, recordSize);
	//
	//AqMD3_ConfigureChannel(*session, channel, range, offset, coupling, VI_TRUE);
	//
	//AqMD3_SetAttributeViInt32(*session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_HYSTERESIS, zsHysteresis);
	//AqMD3_SetAttributeViInt32(*session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_THRESHOLD, zsThreshold);
	//AqMD3_SetAttributeViInt32(*session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_PRE_GATE_SAMPLES, zsPreGateSamples);
	//AqMD3_SetAttributeViInt32(*session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_POST_GATE_SAMPLES, zsPostGateSamples);
	//
	//AqMD3_SetAttributeViString(*session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, triggerSource);
	//AqMD3_SetAttributeViReal64(*session, triggerSource, AQMD3_ATTR_TRIGGER_LEVEL, triggerLevel);
	//AqMD3_SetAttributeViInt32(*session, triggerSource, AQMD3_ATTR_TRIGGER_SLOPE, triggerSlope);
	//
	//AqMD3_ApplySetup(*session);
	//AqMD3_SelfCalibrate(*session);
	//
	//std::unique_ptr<CstZm1Context> context(new CstZm1Context(session, data, markers));

	std::unique_ptr<CstZm1Context> context(nullptr);
	return std::move(context);
}
