#include "../include/digitizer.h"

//std::shared_ptr<ViSession> Digitizer::Configure() {
ViSession &Digitizer::configure() {
	ViSession *session = nullptr;

	ViReal64 const sampleRate = 1.0e9;
	ViReal64 const sampleInterval = 1.0 / sampleRate;
	ViInt64 const recordSize = 1024;
	ViInt32 const streamingMode = AQMD3_VAL_STREAMING_MODE_TRIGGERED;
	ViInt32 const dataReductionMode = AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_ZERO_SUPPRESS;

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

	ViInt64 const nbrSampleElementsToFetch = 3121 * 32 * 32;
	ViInt64 const nbrMarkerElementsToFetch = 64 * 1024;
	ViInt64 const nbrSampleBufferElements = nbrSampleElementsToFetch * 2;
	ViInt64 const nbrMarkerBufferElements = nbrMarkerElementsToFetch * 2;

	AqMD3_InitWithOptions(digitizer, VI_FALSE, VI_FALSE, options, session);

	AqMD3_SetAttributeViInt32(*session, "", AQMD3_ATTR_STREAMING_MODE, streamingMode);
	AqMD3_SetAttributeViReal64(*session, "", AQMD3_ATTR_SAMPLE_RATE, sampleRate);
	AqMD3_SetAttributeViInt32(*session, "", AQMD3_ATTR_ACQUISITION_DATA_REDUCTION_MODE, dataReductionMode);
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

	return *session;
}

enum class MarkersHeader : uint8_t //change name
{
	trigger = 0x01,
	streaming_start_gate = 0x04,
	streaming_stop_gate = 0x05,
};

class TimestampCollection {
public:
	int start_idx;
	int stop_idx;

	std::vector<int> ts_offset_multiple;
	std::vector<int> gs_offset_multiple;

	AcquisitionBuffer& buffer;

	TimestampCollection(int start_index, int stop_index, AcquisitionBuffer& buffer)
		: start_idx(start_index),
		stop_idx(stop_index),
		buffer(buffer),
		ts_offset_multiple(),
		gs_offset_multiple()
	{}

	int get_data_count() // MarkerTag(element & 0xff);
	{
		int32_t *start_addr = buffer.get_raw_at(start_idx);
		for (int32_t *i = start_addr; i < start_addr + stop_idx; i+=2) // move forward 64 bits
		{

		}
	}
};

//AcquiredData* Digitizer::acquire(
//	ViSession& session,
//	AcquisitionBufferPool &data,
//	AcquisitionBufferPool &markers,
//	std::chrono::milliseconds timeout,
//	ViConstString dataStream,
//	ViConstString markerStream,
//	int32_t triggers
//	/*ExcessStructure residual_data_and_markers*/
//	) {
//
//	int trigger_count = 0;
//
//	do
//	{
//		ViInt64 availableElements;
//		ViInt64 actualElementsData;
//		ViInt64 actualElementsMarkers;
//		ViInt64 firstValidElement;
//
//		auto d_buf = data.next_available();
//		auto m_buf = markers.next_available();
//
//		//AqMD3_StreamFetchDataInt32(session, dataStream, d_buf. data.size, (ViInt32*)data.Next(),
//		//	&availableElements, &actualElementsData, &firstValidElement);
//		//AqMD3_StreamFetchDataInt32(session, markerStream, markers.base_size, markers.size, (ViInt32*)markers.Next(),
//		//	&availableElements, &actualElementsMarkers, &firstValidElement);
//
//	} while (trigger_count <= triggers);
//
//	throw std::runtime_error("timeout error");
//}