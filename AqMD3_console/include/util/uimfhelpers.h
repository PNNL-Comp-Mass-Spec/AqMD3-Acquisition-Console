#ifndef UIMF_HELPERS_H
#define UIMF_HELPERS_H

#include <UIMFWriter/uimfframeparameters.h>
#include "../message.pb.h"
#include <climits>
#include <string>
#include <spdlog/spdlog.h>

namespace UIMFHelpers
{
	static UimfFrameParameters uimf_message_to_parameters(const UimfRequestMessage& message)
	{
		return UimfFrameParameters(
			message.start_trigger(),
			message.nbr_samples(),
			message.nbr_accumulations(),
			message.frame_length(),
			message.frame_number(),
			message.offset_bins(),
			message.file_name()
		);
	}

	static UimfFrameParameters create_inf_params()
	{
		return UimfFrameParameters(
			0,
			0,
			0,
			ULLONG_MAX,
			1,
			20000,
			""
		);
	}

	static void log_info_uimf_frame_params(const UimfFrameParameters& message)
	{
		spdlog::info("UimfFrameParameters.frame_number: " + std::to_string(message.frame_number));
		spdlog::info("UimfFrameParameters.frame_length: " + std::to_string(message.frame_length));
		spdlog::info("UimfFrameParameters.nbr_accumulations: " + std::to_string(message.nbr_accumulations));
		spdlog::info("UimfFrameParameters.nbr_samples: " + std::to_string(message.nbr_samples));
		spdlog::info("UimfFrameParameters.offset_bins: " + std::to_string(message.offset_bins));
		spdlog::info("UimfFrameParameters.start_trigger: " + std::to_string(message.start_trigger));
		spdlog::info("UimfFrameParameters.file_name: " + message.file_name);
	}
}

#endif !UIMF_HELPERS_H