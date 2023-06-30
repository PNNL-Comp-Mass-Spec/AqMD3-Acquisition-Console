#ifndef UIMF_HELPERS_H
#define UIMF_HELPERS_H

#include <UIMFWriter/uimfframeparameters.h>
#include "../message.pb.h"
#include <climits>
#include <string>

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
}

#endif !UIMF_HELPERS_H