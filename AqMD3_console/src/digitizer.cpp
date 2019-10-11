#include "../include/digitizer.h"

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_streaming_mode(ViInt32 mode)
{
	ViStatus status = AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_STREAMING_MODE, mode);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_zs_hysteresis(ViConstString channel, ViInt32 hysteresis)
{
	ViStatus status = AqMD3_SetAttributeViInt32(session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_HYSTERESIS, hysteresis);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_zs_threshold(ViConstString channel, ViInt32 threshold)
{
	ViStatus status = AqMD3_SetAttributeViInt32(session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_THRESHOLD, threshold);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_zs_pre_gate_samples(ViConstString channel, ViInt32 samples)
{
	ViStatus status = AqMD3_SetAttributeViInt32(session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_PRE_GATE_SAMPLES, samples);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_zs_post_gate_samples(ViConstString channel, ViInt32 samples)
{
	ViStatus status = AqMD3_SetAttributeViInt32(session, channel, AQMD3_ATTR_CHANNEL_ZERO_SUPPRESS_POST_GATE_SAMPLES, samples);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_channel(ViConstString channel, ViReal64 range, ViReal64 offset, ViInt32 coupling)
{
	ViStatus status = AqMD3_ConfigureChannel(session, channel, range, offset, coupling, VI_TRUE);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_channel_data_inversion(ViConstString channel, ViBoolean invert)
{
	ViStatus status = AqMD3_SetAttributeViBoolean(session, channel, AQMD3_ATTR_CHANNEL_DATA_INVERSION_ENABLED, invert);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::apply_setup()
{
	ViStatus status = AqMD3_ApplySetup(session);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::self_calibrate()
{
	ViStatus status = AqMD3_SelfCalibrate(session);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::begin_acquisition()
{
	ViStatus status = AqMD3_InitiateAcquisition(session);
	return check_error(status);
}

std::pair < std::string, Digitizer:: ErrorType> Digitizer::abort_acquisition()
{
	ViStatus status = AqMD3_Abort(session);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_sample_rate(ViReal64 rate)
{
	ViStatus status = AqMD3_SetAttributeViReal64(session, "", AQMD3_ATTR_SAMPLE_RATE, rate);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_trigger_source(ViConstString trigger_source)
{
	ViStatus status = AqMD3_SetAttributeViString(session, "", AQMD3_ATTR_ACTIVE_TRIGGER_SOURCE, trigger_source);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_trigger_level(ViConstString trigger_source, ViReal64 trigger_level)
{
	ViStatus status = AqMD3_SetAttributeViReal64(session, trigger_source, AQMD3_ATTR_TRIGGER_LEVEL, trigger_level);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_trigger_slope(ViConstString trigger_source, ViInt32 trigger_slope)
{
	ViStatus status = AqMD3_SetAttributeViInt32(session, trigger_source, AQMD3_ATTR_TRIGGER_SLOPE, trigger_slope);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_record_size(ViInt64 record_size)
{
	ViStatus status = AqMD3_SetAttributeViInt64(session, "", AQMD3_ATTR_RECORD_SIZE, record_size);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_data_reduction(ViInt32 mode)
{
	ViStatus status = AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_ACQUISITION_DATA_REDUCTION_MODE, mode);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_acquisition_mode(ViInt32 mode)
{
	ViStatus status = AqMD3_SetAttributeViInt32(session, "", AQMD3_ATTR_ACQUISITION_MODE, mode);
	return check_error(status);
}

std::pair<std::string, Digitizer::ErrorType> Digitizer::configure_io_port(ViConstString port, ViConstString value)
{
	ViStatus status = AqMD3_SetAttributeViString(session, port, AQMD3_ATTR_CONTROL_IO_SIGNAL, value);
	return check_error(status);
}
