#include "../include/libaqmd3/sa220.h"
#include "../include/libaqmd3/cstcontext.h"
#include "../include/libaqmd3/cstzs1context.h"
#include "../include/libaqmd3/helpers.h"
#include <stdexcept>
#include <cstring>

const std::string SA220::channel_1 = "Channel1";
const std::string SA220::channel_2 = "Channel2";
const std::string SA220::samples_channel_1 = "StreamCh1";
const std::string SA220::samples_channel_2 = "StreamCh2";
const std::string SA220::markers_channel_1 = "MarkersCh1";
const std::string SA220::markers_channel_2 = "MarkersCh2";
const std::string SA220::trigger_internal_1 = "Internal1";
const std::string SA220::trigger_internal_2 = "Internal2";
const std::string SA220::trigger_external = "External1";
const std::string SA220::trigger_software = "Software";
const std::string SA220::trigger_self = "SelfTrigger";
const std::string SA220::control_io_1 = "ControlIO1";
const std::string SA220::control_io_2 = "ControlIO2";
const std::string SA220::control_io_3 = "ControlIO3";
const double SA220::full_scale_range_0_5v = 0.5;
const double SA220::full_scale_range_2_5v = 2.5;
const double SA220::max_sample_rate = 2000000000.0;

void SA220::set_sampling_rate(double rate) const
{
	check_and_throw_on_error(configure_sample_rate(rate));
}

void SA220::set_record_size(uint64_t elements) const
{
	check_and_throw_on_error(configure_record_size(elements));
}

void SA220::set_trigger_parameters(std::string trigger, double level, bool isRisingEdgeTriggered, double trigger_delay_s) const
{
	ViInt32 slope = isRisingEdgeTriggered ? AQMD3_VAL_TRIGGER_SLOPE_POSITIVE : AQMD3_VAL_TRIGGER_SLOPE_NEGATIVE;

	check_and_throw_on_error(configure_trigger_source(trigger.c_str()));
	check_and_throw_on_error(configure_trigger_level(trigger.c_str(), level));
	check_and_throw_on_error(configure_trigger_slope(trigger.c_str(), slope));
	check_and_throw_on_error(configure_trigger_delay(trigger_delay_s));
}

void SA220::set_channel_parameters(std::string channel, double range, double offset) const
{
	check_and_throw_on_error(configure_channel(channel.c_str(), range, offset, AQMD3_VAL_VERTICAL_COUPLING_DC));
}

void SA220::set_channel_data_inversion(std::string channel, bool enable) const
{
	check_and_throw_on_error(configure_channel_data_inversion(channel.c_str(), enable));
}


void SA220::enable_io_port(const std::string& io_port) const
{
	std::string enable_string("In-TriggerEnable");
	check_and_throw_on_error(configure_io_port(io_port.c_str(), enable_string.c_str()));
}

void SA220::disable_io_port(const std::string& io_port) const
{
	std::string disable_string("Disabled");
	check_and_throw_on_error(configure_io_port(io_port.c_str(), disable_string.c_str()));
}

bool SA220::get_is_idle() const
{
	auto result = check_and_throw_on_error<int>(Digitizer::get_is_idle());
	return result == AQMD3_VAL_ACQUISITION_STATUS_RESULT_TRUE;
}

std::shared_ptr<StreamingContext> SA220::configure_cst(std::string channel, std::shared_ptr<AcquisitionBufferPool> buffer_pool) const
{
	check_and_throw_on_error(configure_streaming_mode(AQMD3_VAL_STREAMING_MODE_TRIGGERED));
	check_and_throw_on_error(configure_acquisition_mode(AQMD3_VAL_ACQUISITION_MODE_NORMAL));
	check_and_throw_on_error(configure_data_reduction(AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_DISABLED));
	check_and_throw_on_error(apply_setup());

	auto should_calibrate = check_and_throw_on_error<bool>(get_calibration_required());

	if (should_calibrate)
	{
		// TODO: add calibration struct containing last calibration time
		check_and_throw_on_error(self_calibrate());
	}

	return std::make_shared<CstContext>(dynamic_cast<const Digitizer&>(*this), channel, buffer_pool);
}

std::shared_ptr<StreamingContext> SA220::configure_cst(std::string channel, std::shared_ptr<AcquisitionBufferPool> buffer_pool, uint64_t triggers_per_read, ZeroSuppressParameters parameters) const
{
	check_and_throw_on_error(configure_streaming_mode(AQMD3_VAL_STREAMING_MODE_TRIGGERED));
	check_and_throw_on_error(configure_data_reduction(AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_ZERO_SUPPRESS));
	check_and_throw_on_error(configure_zs_hysteresis(channel.c_str(), parameters.hysteresis));
	check_and_throw_on_error(configure_zs_threshold(channel.c_str(), parameters.threshold));
	check_and_throw_on_error(configure_zs_pre_gate_samples(channel.c_str(), parameters.pre_samples));
	check_and_throw_on_error(configure_zs_post_gate_samples(channel.c_str(), parameters.post_samples));
	check_and_throw_on_error(apply_setup());
	auto should_calibrate = check_and_throw_on_error<bool>(get_calibration_required());
	if (should_calibrate)
	{
		check_and_throw_on_error(self_calibrate());
	}

	return std::make_shared<CstZs1Context>(dynamic_cast<const Digitizer&>(*this), channel, triggers_per_read, buffer_pool);
}
