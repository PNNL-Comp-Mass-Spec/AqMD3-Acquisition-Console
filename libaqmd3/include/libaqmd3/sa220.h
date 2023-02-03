#ifndef SA220_H
#define SA220_H

#include "digitizer.h"

class SA220 : private Digitizer {
public:
	 typedef struct Info{
		std::string instrument_model;
		std::string firmware_revision;
		std::string serial_number;
	} Info;

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

	double const full_scale_range_0_5v;
	double const full_scale_range_2_5v;

	double const max_sample_rate;

private:
	Info digitizer_info;
	
	Info get_digitizer_info()
	{
		std::string firmware_rev(256, (char)0);
		Digitizer::get_instrument_firmware_revision(firmware_rev);
		firmware_rev.resize(strlen(firmware_rev.c_str()));

		std::string serial_num(256, (char)0);
		Digitizer::get_instrument_serial_number(serial_num);
		serial_num.resize(strlen(serial_num.c_str()));

		std::string model(256, (char)0);
		Digitizer::get_instrument_model(model);
		serial_num.resize(strlen(model.c_str()));

		return Info{ model, firmware_rev, serial_num };
	}

public:
	SA220(std::string device_handle, bool simulate_device) : Digitizer(device_handle, simulate_device, "SA220P")
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
		, full_scale_range_0_5v(0.5)
		, full_scale_range_2_5v(2.5)
		, max_sample_rate(2000000000.0)
	{
		digitizer_info = get_digitizer_info();
	}

	void set_sampling_rate(double rate) const;
	void set_record_size(uint64_t elements) const;
	void set_trigger_parameters(std::string trigger, double level, bool isRisingEdgeTriggered, double trigger_delay_ms) const;
	void set_channel_parameters(std::string channel, double range, double offset) const;
	void set_channel_data_inversion(std::string channel, bool enable) const;

	Info get_info() const { return digitizer_info; };

	void enable_io_port() const;
	void disable_io_port() const;

	bool get_is_idle() const;

	std::shared_ptr<StreamingContext> configure_cst(std::string channel, std::shared_ptr<AcquisitionBufferPool> buffer_pool) const override;
	std::shared_ptr<StreamingContext> configure_cst(std::string channel, std::shared_ptr<AcquisitionBufferPool>, uint64_t triggers_per_read, ZeroSuppressParameters parameters) const override;
};

#endif // !SA220P
