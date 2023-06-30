#ifndef UIMF_FRAME_PARAMETERS
#define UIMF_FRAME_PARAMETERS

#include <cstdint>
#include <string>

class UimfFrameParameters
{
public:
	uint64_t const start_trigger;
	uint64_t const nbr_samples;
	uint64_t const nbr_accumulations;
	uint64_t const frame_length;
	uint32_t const frame_number;
	uint32_t const offset_bins;
	std::string const file_name;

	UimfFrameParameters(uint64_t start_trigger, uint64_t nbr_samples, uint64_t nbr_accumulations, uint64_t frame_length,
		uint32_t frame_number,
		uint32_t offset_bins,
		std::string file_name)
		: start_trigger(start_trigger)
		, nbr_samples(nbr_samples)
		, nbr_accumulations(nbr_accumulations)
		, frame_length(frame_length)
		, frame_number(frame_number)
		, offset_bins(offset_bins)
		, file_name(file_name)
	{}
};

#endif !UIMF_FRAME_PARAMETERS