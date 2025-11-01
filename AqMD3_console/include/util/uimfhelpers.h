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

	static void log_debug_uimf_frame_params(const UimfFrameParameters& message)
	{
		spdlog::debug("UimfFrameParameters.frame_number: " + std::to_string(message.frame_number));
		spdlog::debug("UimfFrameParameters.frame_length: " + std::to_string(message.frame_length));
		spdlog::debug("UimfFrameParameters.nbr_accumulations: " + std::to_string(message.nbr_accumulations));
		spdlog::debug("UimfFrameParameters.nbr_samples: " + std::to_string(message.nbr_samples));
		spdlog::debug("UimfFrameParameters.offset_bins: " + std::to_string(message.offset_bins));
		spdlog::debug("UimfFrameParameters.start_trigger: " + std::to_string(message.start_trigger));
		spdlog::debug("UimfFrameParameters.file_name: " + message.file_name);
	}
}

#endif !UIMF_HELPERS_H