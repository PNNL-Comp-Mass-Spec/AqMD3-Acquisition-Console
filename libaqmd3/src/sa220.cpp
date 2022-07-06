#include "../include/libaqmd3/sa220.h"
#include "../include/libaqmd3/cstcontext.h"
#include "../include/libaqmd3/cstzs1context.h"
#include <stdexcept>
#include <cstring>

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
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
}

void SA220::set_trigger_parameters(std::string trigger, double level, bool isRisingEdgeTriggered, double trigger_delay_s) const
{
	ViInt32 slope = isRisingEdgeTriggered ? AQMD3_VAL_TRIGGER_SLOPE_POSITIVE : AQMD3_VAL_TRIGGER_SLOPE_NEGATIVE;

	auto result = configure_trigger_source(trigger.c_str());
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
	result = configure_trigger_level(trigger.c_str(), level);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
	result = configure_trigger_slope(trigger.c_str(), slope);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
	result = configure_trigger_delay(trigger_delay_s);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
}

void SA220::set_channel_parameters(std::string channel, double range, double offset) const
{
	auto result = configure_channel(channel.c_str(), range, offset, AQMD3_VAL_VERTICAL_COUPLING_DC);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
}

void SA220::set_channel_data_inversion(std::string channel, bool enable) const
{
	auto result = configure_channel_data_inversion(channel.c_str(), enable);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
}


void SA220::enable_io_port() const
{
	std::string enable_string("In-TriggerEnable");
	auto result = configure_io_port(control_io_2.c_str(), enable_string.c_str());
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
}

void SA220::disable_io_port() const
{
	std::string disable_string("Disabled");
	auto result = configure_io_port(control_io_2.c_str(), disable_string.c_str());
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
}

bool SA220::get_is_idle() const
{
	auto result = Digitizer::get_is_idle();
	if (std::get<1>(result) == Digitizer::None)
	{
		throw std::runtime_error(std::get<0>(result));
	}

	return std::get<2>(result) == AQMD3_VAL_ACQUISITION_STATUS_RESULT_TRUE;
}

std::shared_ptr<StreamingContext> SA220::configure_cst(std::string channel, uint32_t triggers, uint64_t record_size, uint32_t preallocated_buffers) const
{
	auto result = configure_streaming_mode(AQMD3_VAL_STREAMING_MODE_TRIGGERED);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}

	result = configure_acquisition_mode(AQMD3_VAL_ACQUISITION_MODE_NORMAL);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}

	result = configure_data_reduction(AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_DISABLED);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}

	result = apply_setup();
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}

	auto rct = get_calibration_required();
	if (std::get<1>(rct) == Digitizer::None)
	{
		throw std::runtime_error(std::get<0>(rct));
	}

	bool should_calibrate = (bool)std::get<2>(rct);
	if (should_calibrate)
	{
		// TODO: add calibration struct containing last calibration time
		result = self_calibrate();
		if (result.second == Digitizer::Error)
		{
			throw std::runtime_error(result.first);
		}
	}

	return std::make_shared<CstContext>(dynamic_cast<const Digitizer&>(*this), channel, record_size * 64, preallocated_buffers, record_size, triggers);
}

std::shared_ptr<StreamingContext> SA220::configure_cst_zs1(std::string channel, uint32_t triggers, uint64_t record_size, ZeroSuppressParameters parameters, uint32_t preallocated_buffers) const
{
	auto result = configure_streaming_mode(AQMD3_VAL_STREAMING_MODE_TRIGGERED);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
	result = configure_data_reduction(AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_ZERO_SUPPRESS);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
	result = configure_zs_hysteresis(channel.c_str(), parameters.hysteresis);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
	result = configure_zs_threshold(channel.c_str(), parameters.threshold);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
	result = configure_zs_pre_gate_samples(channel.c_str(), parameters.pre_samples);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
	result = configure_zs_post_gate_samples(channel.c_str(), parameters.post_samples);
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
	result = apply_setup();
	if (result.second == Digitizer::Error)
	{
		throw std::runtime_error(result.first);
	}
	auto rct = get_calibration_required();
	if (std::get<1>(rct) == Digitizer::None)
	{
		throw std::runtime_error(std::get<0>(rct));
	}
	bool should_calibrate = (bool)std::get<2>(rct);
	if (should_calibrate)
	{
		result = self_calibrate();
		if (result.second == Digitizer::Error)
		{
			throw std::runtime_error(result.first);
		}
	}

	return std::make_shared<CstZs1Context>(dynamic_cast<const Digitizer&>(*this), channel, record_size * 256, preallocated_buffers, record_size, triggers);
}
