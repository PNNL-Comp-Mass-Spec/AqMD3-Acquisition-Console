#ifndef DIGITIZER_H
#define DIGITIZER_H

#include "streamingcontext.h"
#include "acquireddata.h"
#include "AqMD3.h"
#include <tuple>
#include <memory>
#include <chrono>
#include <string>
#include <format>
#include <string>

class Digitizer {
public:
	enum ErrorType {
		None,
		Warning,
		Error,
	};

	struct ZeroSuppressParameters {
		int16_t const threshold;
		uint16_t const hysteresis;
		uint8_t const pre_samples;
		uint8_t const post_samples;

		ZeroSuppressParameters(int16_t threshold, uint16_t hysteresis)
			: threshold(threshold)
			, hysteresis(hysteresis)
			, pre_samples(0)
			, post_samples(0)
		{}
	};

protected:
	ViSession session;

public: 
	Digitizer(std::string resource, bool simulate_device, std::string device_type) 
		: session(VI_NULL)
	{
		std::string options = std::format("Simulate={}, DriverSetup= Model={}", simulate_device ? "true" : "false", device_type);
		AqMD3_InitWithOptions((ViChar *)resource.c_str(), VI_FALSE, VI_FALSE, options.c_str(), &session);
	}
	~Digitizer()
	{
		AqMD3_close(session);
	}

	virtual std::shared_ptr<StreamingContext> configure_cst(std::string channel, std::shared_ptr<AcquisitionBufferPool> buffer_pool) const = 0;
	virtual std::shared_ptr<StreamingContext> configure_cst(std::string channel, std::shared_ptr<AcquisitionBufferPool> buffer_pool, uint64_t triggers_per_read, ZeroSuppressParameters parameters) const = 0;

private:
	std::pair<std::string, ErrorType> check_error(ViStatus status) const 
	{
		
		ViInt32 ec;
		ViChar error_message[512];

		if (status > 0)
		{
			AqMD3_GetError(VI_NULL, &ec, sizeof(error_message), error_message);
			return std::pair<std::string, ErrorType>("Error Code: " + std::to_string(ec) + "Error Message: " + error_message, Warning);

		}
		else if (status < 0)
		{
			AqMD3_GetError(VI_NULL, &ec, sizeof(error_message), error_message);
			return std::pair<std::string, ErrorType>("Error Code: " + std::to_string(ec) + "Error Message: " + error_message, Error);
		}

		return std::pair<std::string, ErrorType>("", None);
	}
	
public:
	std::pair<std::string, ErrorType> get_instrument_options_info(std::string &options) const;
	std::pair<std::string, ErrorType> get_instrument_model(std::string &model) const;
	std::pair<std::string, ErrorType> get_instrument_serial_number(std::string &serial_number) const;
	std::pair<std::string, ErrorType> get_instrument_firmware_revision(std::string &firmware_revision) const;

	std::pair<std::string, ErrorType> configure_streaming_mode(ViInt32 mode) const;
	std::pair<std::string, ErrorType> configure_sample_rate(ViReal64 rate) const;

	std::pair<std::string, ErrorType> configure_trigger_source(ViConstString trigger_source) const;
	std::pair<std::string, ErrorType> configure_trigger_level(ViConstString trigger_source, ViReal64 trigger_level) const;
	std::pair<std::string, ErrorType> configure_trigger_slope(ViConstString trigger_source, ViInt32 trigger_slope) const;
	std::pair<std::string, ErrorType> configure_trigger_delay(ViReal64 delay) const;

	std::pair<std::string, ErrorType> configure_record_size(ViInt64 record_size) const;
	std::pair<std::string, ErrorType> configure_data_reduction(ViInt32 mode) const;
	std::pair<std::string, ErrorType> configure_acquisition_mode(ViInt32 mode) const;

	std::pair<std::string, ErrorType> configure_zs_hysteresis(ViConstString channel, ViInt32 hysteresis) const;
	std::pair<std::string, ErrorType> configure_zs_threshold(ViConstString channel, ViInt32 threshold) const;
	std::pair<std::string, ErrorType> configure_zs_pre_gate_samples(ViConstString channel, ViInt32 samples) const;
	std::pair<std::string, ErrorType> configure_zs_post_gate_samples(ViConstString channel, ViInt32 samples) const;

	std::pair<std::string, ErrorType> configure_channel(ViConstString channel, ViReal64 range, ViReal64 offset, ViInt32 coupling) const;
	std::pair<std::string, ErrorType> configure_channel_data_inversion(ViConstString channel, ViBoolean invert) const;

	std::pair<std::string, ErrorType> configure_io_port(ViConstString port, ViConstString value) const;

	std::pair<std::string, ErrorType> apply_setup() const;
	std::pair<std::string, ErrorType> self_calibrate() const;

	std::pair<std::string, ErrorType> begin_acquisition() const;
	std::pair<std::string, ErrorType> abort_acquisition() const;

	std::pair<std::string, ErrorType> stream_fetch_data(ViConstString stream, ViInt64 elements_to_Fetch, ViInt64 buffer_size, ViInt32 buffer[], ViInt64* available_elements, ViInt64* actual_elements, ViInt64* first_valid_element_index) const;

	std::tuple<std::string, ErrorType, ViBoolean> get_calibration_required() const;

	std::tuple<std::string, ErrorType, ViInt32> get_is_idle() const;
};

#endif // ! DIGITIZER_H
