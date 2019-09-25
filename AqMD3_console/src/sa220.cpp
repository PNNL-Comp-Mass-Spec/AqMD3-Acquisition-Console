#include "../include/sa220.h"

#include "../include/cstcontext.h"
#include "../include/cstzs1context.h"

void SA220::set_sampling_rate(double rate)
{
	auto result = configure_sample_rate(rate);
	cout << result.first << " " << result.second << endl;
}

void SA220::set_record_size(uint64_t elements)
{
	auto result = configure_record_size(record_size);
	if (result.second != Digitizer::None)
	{
		cerr << result.first << endl;
		return;
	}

	record_size = elements;
}

void SA220::set_trigger_parameters(std::string trigger, double level, bool isRising)
{
	ViInt32 slope = isRising ? AQMD3_VAL_TRIGGER_SLOPE_POSITIVE : AQMD3_VAL_TRIGGER_SLOPE_NEGATIVE;

	auto tso_result = configure_trigger_source(trigger.c_str());
	auto tl_result = configure_trigger_level(trigger.c_str(), level);
	auto tsl_result = configure_trigger_slope(trigger.c_str(), slope);
}

void SA220::enable_io_port()
{
	std::string enable_string("In-TriggerEnable");
	auto result = configure_io_port(control_io_2.c_str(), enable_string.c_str());
}

void SA220::disable_io_port()
{
	std::string disable_string("Disabled");
	auto result = configure_io_port(control_io_2.c_str(), disable_string.c_str());
}

std::shared_ptr<StreamingContext> SA220::configure_cst(std::string samples_channel, std::string markers_channel)
{
	// Acquisition configuration parameters
	ViReal64 const sampleRate = 1.0e9;
	ViInt64 const recordSize = 94016;
	ViInt32 const streamingMode = AQMD3_VAL_STREAMING_MODE_TRIGGERED;
	ViInt32 const acquisitionMode = AQMD3_VAL_ACQUISITION_MODE_NORMAL;

	ViConstString channel = "Channel1";
	ViReal64 const range = 2.5;
	ViReal64 const offset = 0.0;
	ViInt32 const coupling = AQMD3_VAL_VERTICAL_COUPLING_DC;

	// Trigger configuration
	ViConstString triggerSource = "External1";
	ViReal64 const triggerLevel = 0.50;
	ViInt32 const triggerSlope = AQMD3_VAL_TRIGGER_SLOPE_POSITIVE;

	ViConstString sampleStreamName = "StreamCh1";
	ViConstString markerStreamName = "MarkersCh1";
	ViInt64 const nbrOfRecordsToFetchAtOnce = 256;

	AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_STREAMING_MODE, streamingMode);
	AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate);
	AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_ACQUISITION_MODE, acquisitionMode);
	AqMD3_SetAttributeViInt64(session, "", AQMD3_ATTR_RECORD_SIZE, recordSize);
	AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_ACQUISITION_DATA_REDUCTION_MODE, AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_DISABLED);


	AqMD3_ConfigureChannel(session, channel, range, offset, coupling, VI_TRUE);

	AqMD3_SetAttributeViString(session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, triggerSource);
	AqMD3_SetAttributeViReal64(session, triggerSource, AQMD3_ATTR_TRIGGER_LEVEL, triggerLevel);
	AqMD3_SetAttributeViInt32(session, triggerSource, AQMD3_ATTR_TRIGGER_SLOPE, triggerSlope);

	AqMD3_ApplySetup(session);
	AqMD3_SelfCalibrate(session);

	ViInt64 sampleStreamGrain = 0;
	ViInt64 markerStreamGrain = 0;
	AqMD3_GetAttributeViInt64(session, sampleStreamName, AQMD3_ATTR_STREAM_GRANULARITY_IN_BYTES, &sampleStreamGrain);
	AqMD3_GetAttributeViInt64(session, markerStreamName, AQMD3_ATTR_STREAM_GRANULARITY_IN_BYTES, &markerStreamGrain);
	ViInt64 const sampleStreamGrainElements = sampleStreamGrain / sizeof(int32_t);
	ViInt64 const markerStreamGrainElements = markerStreamGrain / sizeof(int32_t);

	ViInt64 const nbrSamplesPerElement = 2;

	ViInt64 const recordElements = recordSize / nbrSamplesPerElement;
	ViInt64 const acquisitionElements = recordElements * nbrOfRecordsToFetchAtOnce;
	ViInt64 const markerElements = 16 * nbrOfRecordsToFetchAtOnce;

	ViInt64 const sampleStreamBufferSize = acquisitionElements	// required elements
		+ acquisitionElements / 2								// unwrapping overhead (only in single channel mode)
		+ sampleStreamGrainElements - 1;						// alignment overhead

	ViInt64 const markerStreamBufferSize = markerElements	// required elements
		+ markerStreamGrainElements - 1;					// alignment overhead

	AcquisitionBufferPool *data = new AcquisitionBufferPool(sampleStreamBufferSize, 8);
	AcquisitionBufferPool *markers = new AcquisitionBufferPool(markerStreamBufferSize, 8);

	ViReal64 sampleRateCheck = 0.0;
	AqMD3_GetAttributeViReal64(session, "", AQMD3_ATTR_SAMPLE_RATE, &sampleRateCheck);

	std::shared_ptr<CstContext> context(new CstContext(session, data, markers, markerStreamName, sampleStreamName, recordSize));

	return context;
}

//std::shared_ptr<StreamingContext> SA220::configure_cst_zs1(std::string samples_channel, std::string markers_channel, int16_t threshold, uint16_t hysteresis, uint8_t pre_samples, uint8_t post_samples)
std::shared_ptr<StreamingContext> SA220::configure_cst_zs1(std::string channel, int16_t threshold, uint16_t hysteresis, uint8_t pre_samples, uint8_t post_samples)
{
	//ViReal64 const sampleRate = 1.0e9;
	//ViInt64 const recordSize = 94016;
	//ViInt32 const streamingMode = AQMD3_VAL_STREAMING_MODE_TRIGGERED;
	//ViInt32 const dataReductionMode = AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_ZERO_SUPPRESS;

	// 1. call SA220 method
	//ViConstString triggerSource = "External1";
	//ViReal64 const triggerLevel = 0.50;
	//ViInt32 const triggerSlope = AQMD3_VAL_TRIGGER_SLOPE_POSITIVE;

	ViInt64 const nbrSampleElementsToFetch = 94016 * 256;					// enough to cover
	ViInt64 const nbrMarkerElementsToFetch = 16 * 256 * 256;				// all cases
	ViInt64 const nbrSampleBufferElements = nbrSampleElementsToFetch * 4;	// and then add
	ViInt64 const nbrMarkerBufferElements = nbrMarkerElementsToFetch * 4;	// even more

	// call dig method
	//AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_STREAMING_MODE, streamingMode);
	auto rc = configure_streaming_mode(AQMD3_VAL_STREAMING_MODE_TRIGGERED);
	
	//AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate);
	
	// call dig method
	//AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_ACQUISITION_DATA_REDUCTION_MODE, dataReductionMode);
	rc = configure_data_reduction(AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_ZERO_SUPPRESS);

	// 2. call SA220 method
	//AqMD3_SetAttributeViInt64(session, "", AQMD3_ATTR_RECORD_SIZE, recordSize);

	//ViReal64 const range = 2.5;
	//ViReal64 const offset = 0.0;
	//ViConstString channel = "Channel1";
	// call SA220 method
	//AqMD3_ConfigureChannel(session, channel.c_str(), range, offset, AQMD3_VAL_VERTICAL_COUPLING_DC, VI_TRUE);

	//call dig method
	//AqMD3_SetAttributeViInt32(session, channel.c_str(), AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_HYSTERESIS, hysteresis);
	//AqMD3_SetAttributeViInt32(session, channel.c_str(), AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_THRESHOLD, threshold);
	rc = configure_zs_hysteresis(channel.c_str(), hysteresis);
	rc = configure_zs_threshold(channel.c_str(), threshold);

	// 3. call SA220 method
	//AqMD3_SetAttributeViString(session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, triggerSource);
	//AqMD3_SetAttributeViReal64(session, triggerSource, AQMD3_ATTR_TRIGGER_LEVEL, triggerLevel);
	//AqMD3_SetAttributeViInt32(session, triggerSource, AQMD3_ATTR_TRIGGER_SLOPE, triggerSlope);

	ViInt64 sampleStreamGrain = 0;
	ViInt64 markerStreamGrain = 0;
	AqMD3_GetAttributeViInt64(session, "StreamCh1", AQMD3_ATTR_STREAM_GRANULARITY_IN_BYTES, &sampleStreamGrain);
	AqMD3_GetAttributeViInt64(session, "MarkersCh1", AQMD3_ATTR_STREAM_GRANULARITY_IN_BYTES, &markerStreamGrain);

	AqMD3_ApplySetup(session);
	AqMD3_SelfCalibrate(session);

	AcquisitionBufferPool *data = new AcquisitionBufferPool(nbrSampleBufferElements, 8);
	AcquisitionBufferPool *markers = new AcquisitionBufferPool(nbrMarkerBufferElements, 2);

	std::shared_ptr<CstZm1Context> context(new CstZm1Context(session, data, markers, "MarkersCh1", "StreamCh1", markerStreamGrain, sampleStreamGrain));

	return std::move(context);
}
