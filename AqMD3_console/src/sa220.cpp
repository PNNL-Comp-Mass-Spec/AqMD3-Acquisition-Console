#include "../include/sa220.h"

#include "../include/cstcontext.h"
#include "../include/cstzs1context.h"
#include <exception>

void SA220::set_sampling_rate(double rate) const
{
	auto result = configure_sample_rate(rate);
	if (result.second != Digitizer::None)
	{
		std::cerr << result.first << std::endl;
		return;
	}
}

void SA220::set_record_size(uint64_t elements) const
{
	auto result = configure_record_size(elements);
	if (result.second != Digitizer::None)
	{
		std::cerr << result.first << std::endl;
		return;
	}
}

void SA220::set_trigger_parameters(std::string trigger, double level, bool isRisingEdgeTriggered, double trigger_delay_s) const
{
	ViInt32 slope = isRisingEdgeTriggered ? AQMD3_VAL_TRIGGER_SLOPE_POSITIVE : AQMD3_VAL_TRIGGER_SLOPE_NEGATIVE;

	auto tso_result = configure_trigger_source(trigger.c_str());
	if (tso_result.second != Digitizer::None)
	{
		std::cerr << tso_result.first << std::endl;
		return;
	}
	auto tl_result = configure_trigger_level(trigger.c_str(), level);
	if (tl_result.second != Digitizer::None)
	{
		std::cerr << tl_result.first << std::endl;
		return;
	}
	auto tsl_result = configure_trigger_slope(trigger.c_str(), slope);
	if (tsl_result.second != Digitizer::None)
	{
		std::cerr << tsl_result.first << std::endl;
		return;
	}

	auto td_result = configure_trigger_delay(trigger_delay_s);
}

void SA220::set_channel_parameters(std::string channel, double range, double offset) const
{
	auto result = configure_channel(channel.c_str(), range, offset, AQMD3_VAL_VERTICAL_COUPLING_DC);
	if (result.second != Digitizer::None)
	{
		std::cerr << result.first << std::endl;
		return;
	}
}

void SA220::set_channel_data_inversion(std::string channel, bool enable) const
{
	auto result = configure_channel_data_inversion(channel.c_str(), enable);
	if (result.second != Digitizer::None)
	{
		std::cerr << result.first << std::endl;
		return;
	}
}

std::string SA220::get_device_info() const
{
	std::string str;
	str.reserve(512);

	auto result = get_instrument_options_info(&str[0]);
	if (result.second != Digitizer::None)
	{
		std::cerr << result.first << std::endl;
		return "";
	}

	return str;
}

void SA220::enable_io_port() const
{
	std::string enable_string("In-TriggerEnable");
	auto result = configure_io_port(control_io_2.c_str(), enable_string.c_str());
	if (result.second != Digitizer::None)
	{
		std::cerr << result.first << std::endl;
		return;
	}
}

void SA220::disable_io_port() const
{
	std::string disable_string("Disabled");
	auto result = configure_io_port(control_io_2.c_str(), disable_string.c_str());
	if (result.second != Digitizer::None)
	{
		std::cerr << result.first << std::endl;
		return;
	}
}

std::unique_ptr<StreamingContext> SA220::configure_cst(std::string channel, uint32_t triggers, uint64_t record_size) const
{
	auto rc = configure_streaming_mode(AQMD3_VAL_STREAMING_MODE_TRIGGERED);
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_acquisition_mode(AQMD3_VAL_ACQUISITION_MODE_NORMAL);
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_data_reduction(AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_DISABLED);
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = apply_setup();
	if (rc.second != Digitizer::None)
		throw rc.first;

	auto rct = get_calibration_required();
	if (std::get<1>(rct) != Digitizer::None)
		throw rc.first;

	bool should_calibrate = (bool)std::get<2>(rct);
	if (should_calibrate)
	{
		rc = self_calibrate();
		if (rc.second != Digitizer::None)
			throw rc.first;
	}

	return std::make_unique<CstContext>(dynamic_cast<const Digitizer&>(*this), channel, record_size * 64, 8, record_size, triggers);
}

std::unique_ptr<StreamingContext> SA220::configure_cst_zs1(std::string channel, uint32_t triggers, uint64_t record_size, ZeroSuppressParameters parameters) const
{
	auto rc = configure_streaming_mode(AQMD3_VAL_STREAMING_MODE_TRIGGERED);

	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_data_reduction(AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_ZERO_SUPPRESS);
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_zs_hysteresis(channel.c_str(), parameters.hysteresis);
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_zs_threshold(channel.c_str(), parameters.threshold);
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_zs_pre_gate_samples(channel.c_str(), parameters.pre_samples);
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_zs_post_gate_samples(channel.c_str(), parameters.post_samples);
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = apply_setup();
	if (rc.second != Digitizer::None)
		throw rc.first;

	auto rct = get_calibration_required();
	if (std::get<1>(rct) != Digitizer::None)
		throw rc.first;

	bool should_calibrate = (bool)std::get<2>(rct);
	if (should_calibrate)
	{
		rc = self_calibrate();
		if (rc.second != Digitizer::None)
			throw rc.first;
	}

	return std::make_unique<CstZm1Context>(dynamic_cast<const Digitizer&>(*this), channel, record_size * 256, 2, record_size, triggers);
}
