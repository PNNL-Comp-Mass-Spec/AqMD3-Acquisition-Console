#ifndef SA220_H
#define SA220_H

#include "digitizer.h"

class SA220 : public Digitizer {
public:
	std::string const channel_1;
	std::string const channel_2;

	std::string const samples_channel_1;
	std::string const samples_channel_2;
	std::string const markers_channel_1;
	std::string const markers_channel_2;

	std::string const trigger_internal_1;
	std::string const trigger_internal_2;
	std::string const trigger_external;
	std::string const trigger_software;
	std::string const trigger_self;

	std::string const control_io_1;
	std::string const control_io_2;
	std::string const control_io_3;

	std::string const visa_device_id;
	std::string const options;

	double const full_scale_range_500mv;
	double const full_scale_range_2500mv;

private:
	uint64_t record_size;

public:
	SA220(std::string device, std::string options) : Digitizer(device, options)
		, channel_1("Channel1")
		, channel_2("Channel2")
		, samples_channel_1("StreamCh1")
		, samples_channel_2("StreamCh2")
		, markers_channel_1("MarkersCh1")
		, markers_channel_2("MarkersCh2")
		, trigger_internal_1("Internal1")
		, trigger_internal_2("Internal2")
		, trigger_external("External1")
		, trigger_software("Software")
		, trigger_self("SelfTrigger")
		, control_io_1("ControlIO1")
		, control_io_2("ControlIO2")
		, control_io_3("ControlIO3")
		, full_scale_range_500mv(0.5)
		, full_scale_range_2500mv(2.5)
	{}

	void set_sampling_rate(double rate);
	void set_record_size(uint64_t elements);
	void set_trigger_parameters(std::string trigger, double level, bool isRisingEdgeTriggered);
	void set_channel_parameters(std::string channel, double range, double offset);

	void enable_io_port();
	void disable_io_port();

	std::shared_ptr<StreamingContext> configure_cst(std::string channel, uint32_t triggers) override;
	std::shared_ptr<StreamingContext> configure_cst_zs1(std::string channel, uint32_t triggers, ZeroSuppressParameters parameters) override;
};

#endif // !SA220P