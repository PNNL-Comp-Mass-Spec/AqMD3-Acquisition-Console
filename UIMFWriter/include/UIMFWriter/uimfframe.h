#ifndef UIMF_FRAME_H
#define UIMF_FRAME_H

#include "uimfframeparameters.h"
#include "encodedresult.h"
#include <string>
#include <vector>
#include <list>
#include <chrono>
#include <memory>

class UimfFrame {
private:
	std::string const time_created;
	UimfFrameParameters const frame_parameters;
	std::vector<EncodedResult> encoded_data;

	//std::vector<std::shared_ptr<std::vector<EncodedResult>>> data;

	//UimfFrame(uint64_t start_trigger, uint64_t nbr_samples, uint64_t nbr_accumulations, uint64_t frame_length,
	//	uint32_t frame_number,
	//	uint32_t offset_bins,
	//	std::string file_name,
	//	std::vector<EncodedResult> data)
	//	: start_trigger(start_trigger)
	//	, nbr_samples(nbr_samples)
	//	, nbr_accumulations(nbr_accumulations)
	//	, frame_length(frame_length)
	//	, frame_number(frame_number)
	//	, offset_bins(offset_bins)
	//	, file_name(file_name)
	//	, data(data)
	//{}

public:
	UimfFrame(UimfFrameParameters frame_parameters, std::vector<EncodedResult> encoded_data)
		: frame_parameters(frame_parameters)
		, encoded_data(encoded_data)
	{}

	~UimfFrame() {}

	//inline std::shared_ptr<UimfFrame> clone() { return std::make_shared<UimfFrame>(start_trigger, nbr_samples, nbr_accumulations, frame_length, frame_number, offset_bins, file_name); }
	//inline const std::vector<std::shared_ptr<std::vector<EncodedResult>>> get_data() const { return data; }
	//void append_encoded_results(std::shared_ptr<std::vector<EncodedResult>> results);
	//std::shared_ptr<std::vector<EncodedResult>> append_and_return_excess(std::shared_ptr<std::vector<EncodedResult>> results);
	//int get_encoded_results_count() const;
	//std::pair<int32_t, int32_t> get_encoded_results_scan_range() const;
	double get_frame_duration_seconds(double sample_period) const;
	const std::vector<EncodedResult>& data() const { return encoded_data; }
	const UimfFrameParameters& parameters() const { return frame_parameters; }
};

#endif // !UIMF_FRAME_H