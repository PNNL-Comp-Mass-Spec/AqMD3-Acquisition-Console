#include "../include/sa220.h"

#include "../include/cstcontext.h"
#include "../include/cstzs1context.h"
#include <exception>

void SA220::set_sampling_rate(double rate)
{
	auto result = configure_sample_rate(rate);
	cout << result.first << " " << result.second << endl;
}

void SA220::set_record_size(uint64_t elements)
{
	auto result = configure_record_size(elements);
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

void SA220::set_channel_parameters(std::string channel, double range, double offset)
{
	auto result = configure_channel(channel.c_str(), range, offset, AQMD3_VAL_VERTICAL_COUPLING_DC);
	cout << result.first << " " << result.second << endl;
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

std::unique_ptr<StreamingContext> SA220::configure_cst(std::string channel, uint32_t triggers)
{
	auto rc = configure_streaming_mode(AQMD3_VAL_STREAMING_MODE_TRIGGERED);
	cout << rc.first << " " << rc.second << endl;
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_acquisition_mode(AQMD3_VAL_ACQUISITION_MODE_NORMAL);
	cout << rc.first << " " << rc.second << endl;
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_data_reduction(AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_DISABLED);
	cout << rc.first << " " << rc.second << endl;
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = apply_setup();
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = self_calibrate();
	if (rc.second != Digitizer::None)
		throw rc.first;

	std::unique_ptr<CstContext> context(
		new CstContext
		(
			session,
			"StreamCh1",
			record_size * 64,
			8,
			"MarkersCh1",
			record_size,
			triggers
		));

	return std::move(context);
}

std::unique_ptr<StreamingContext> SA220::configure_cst_zs1(std::string channel, uint32_t triggers, ZeroSuppressParameters parameters)
{
	auto rc = configure_streaming_mode(AQMD3_VAL_STREAMING_MODE_TRIGGERED);

	cout << rc.first << " " << rc.second << endl;
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_data_reduction(AQMD3_VAL_ACQUISITION_DATA_REDUCTION_MODE_ZERO_SUPPRESS);
	cout << rc.first << " " << rc.second << endl;
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_zs_hysteresis(channel.c_str(), parameters.hysteresis);
	cout << rc.first << " " << rc.second << endl;
	if (rc.second != Digitizer::None)
		throw rc.first;

	rc = configure_zs_threshold(channel.c_str(), parameters.threshold);
	cout << rc.first << " " << rc.second << endl;
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

	rc = self_calibrate();
	if (rc.second != Digitizer::None)
		throw rc.first;

	std::unique_ptr<CstZm1Context> context(
		new CstZm1Context
		(
			session,
			"StreamCh1",
			record_size * 256,
			8,
			"MarkersCh1",
			256 * 256 * 4,
			3,
			record_size,
			triggers
		));

	return std::move(context);
}
