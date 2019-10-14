#ifndef DIGITIZER_H
#define DIGITIZER_H

#include "streamingcontext.h"
#include "acquireddata.h"
#include "acquisitioncontext.h"
#include "AqMD3.h"
#include <tuple>
#include <memory>
#include <chrono>
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

		ZeroSuppressParameters(int16_t threshold, uint16_t hysteresis, uint8_t pre_samples, uint8_t post_samples)
			: threshold(threshold)
			, hysteresis(hysteresis)
			, pre_samples(pre_samples)
			, post_samples(post_samples)
		{}
	};

protected:
	ViSession session;

public: 
	Digitizer(std::string resource, std::string options) 
		: session(VI_NULL)
	{
		AqMD3_InitWithOptions((ViChar *)resource.c_str(), VI_FALSE, VI_FALSE, options.c_str(), &session);
	}
	~Digitizer()
	{
		AqMD3_close(session);
	}

	virtual std::unique_ptr<StreamingContext> configure_cst(std::string channel, uint32_t triggers, uint64_t record_size) = 0;
	virtual std::unique_ptr<StreamingContext> configure_cst_zs1(std::string channel, uint32_t triggers, uint64_t record_size, ZeroSuppressParameters parameters) = 0;

private:
	std::pair<std::string, ErrorType> check_error(ViStatus status) {
		
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
	
protected:
	std::pair<std::string, ErrorType> configure_streaming_mode(ViInt32 mode);
	std::pair<std::string, ErrorType> configure_sample_rate(ViReal64 rate);

	std::pair<std::string, ErrorType> configure_trigger_source(ViConstString trigger_source);
	std::pair<std::string, ErrorType> configure_trigger_level(ViConstString trigger_source, ViReal64 trigger_level);
	std::pair<std::string, ErrorType> configure_trigger_slope(ViConstString trigger_source, ViInt32 trigger_slope);

	std::pair<std::string, ErrorType> configure_record_size(ViInt64 record_size);
	std::pair<std::string, ErrorType> configure_data_reduction(ViInt32 mode);
	std::pair<std::string, ErrorType> configure_acquisition_mode(ViInt32 mode);

	std::pair<std::string, ErrorType> configure_zs_hysteresis(ViConstString channel, ViInt32 hysteresis);
	std::pair<std::string, ErrorType> configure_zs_threshold(ViConstString channel, ViInt32 threshold);
	std::pair<std::string, ErrorType> configure_zs_pre_gate_samples(ViConstString channel, ViInt32 samples);
	std::pair<std::string, ErrorType> configure_zs_post_gate_samples(ViConstString channel, ViInt32 samples);

	std::pair<std::string, ErrorType> configure_channel(ViConstString channel, ViReal64 range, ViReal64 offset, ViInt32 coupling);
	std::pair<std::string, ErrorType> configure_channel_data_inversion(ViConstString channel, ViBoolean invert);

	std::pair<std::string, ErrorType> configure_io_port(ViConstString port, ViConstString value);

	std::pair<std::string, ErrorType> apply_setup();
	std::pair<std::string, ErrorType> self_calibrate();

	std::pair<std::string, ErrorType> begin_acquisition();
	std::pair<std::string, ErrorType> abort_acquisition();

	std::tuple<std::string, ErrorType, ViBoolean> get_calibration_required();
};

#endif // ! DIGITIZER_H
