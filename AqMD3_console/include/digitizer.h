#ifndef DIGITIZER_H
#define DIGITIZER_H

#include "streamingcontext.h"
#include "acquisitionbufferpool.h"
#include "acquireddata.h"
#include "acquisitioncontext.h"
#include "AqMD3.h"
#include <tuple>
#include <memory>
#include <chrono>
#include <string>

class Digitizer {
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

	std::shared_ptr<StreamingContext> configure_cst(std::string samples_channel, std::string markers_channel);
	std::shared_ptr<StreamingContext> configure_cst_zs1(std::string samples_channel, std::string markers_channel, int16_t threshold, uint16_t hysteresis = 100);

protected:
	std::pair<std::string, bool> check_call(ViStatus&& status) {
		
		ViInt32 ec;
		ViChar error_message[512];

		if (status > 0)
		{
			AqMD3_GetError(VI_NULL, &ec, sizeof(error_message), error_message);
			return std::pair<std::string, bool>("Error Code: " + std::to_string(ec) + "Error Message: " + error_message, false);

		}
		else if (status < 0)
		{
			AqMD3_GetError(VI_NULL, &ec, sizeof(error_message), error_message);
			return std::pair<std::string, bool>("Error Code: " + std::to_string(ec) + "Error Message: " + error_message, true);
		}
	}

};

#endif // ! DIGITIZER_H
