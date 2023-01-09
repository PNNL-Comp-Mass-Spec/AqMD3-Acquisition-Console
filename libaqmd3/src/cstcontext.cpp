#include "../include/libaqmd3/cstcontext.h"
#include "../include/libaqmd3/acquisitionbuffer.h"
#include "../include/libaqmd3/digitizer.h"

#include <iostream>
#include <stdexcept>

AcquiredData CstContext::acquire(std::chrono::milliseconds timeoutMs)
{
	std::shared_ptr<AcquisitionBuffer> samples_buffer = buffer_pool->get_buffer();
	auto triggers_per_read = samples_buffer->get_triggers_per_read();
	auto samples_per_trigger = samples_buffer->get_samples_per_trigger();
	auto markers_to_acquire = triggers_per_read * 16;
	std::vector<int32_t> markers_buffer(markers_to_acquire * 16);

	ViInt64 first_element_markers;
	ViInt64 available_elements_markers = 0;
	ViInt64 actual_elements_markers = 0;

	do
	{
		auto rc = digitizer.stream_fetch_data(
			markers_channel.c_str(),
			markers_to_acquire,
			markers_buffer.size(),
			(ViInt32 *)markers_buffer.data(),
			&available_elements_markers, &actual_elements_markers, &first_element_markers);
		if (rc.second == Digitizer::Error)
		{
			throw std::runtime_error(rc.first);
		}
	} while (actual_elements_markers < markers_to_acquire);

	std::vector<AcquiredData::TriggerData> stamps;

	int32_t *ptr = markers_buffer.data() + first_element_markers;
	int32_t offset = 0;
	for (int i = 0; i < triggers_per_read; i++)
	{
		int32_t *seg = ptr + offset;
		uint32_t header = seg[0];

		if ((header & 0x000000FF) != 0x01)
			std::cerr << "wrong header -- cst acq (not zero sp)\n";

		uint64_t low = seg[1];
		uint64_t high = seg[2];
		uint64_t timestampLow = (low >> 8) & 0x0000000000ffffffL;
		uint64_t timestampHigh = uint64_t(high) << 24;
		stamps.emplace_back(timestampHigh | timestampLow, header >> 8, (-1 * (seg[1] & 0x000000ff)) / 256);

		offset += 16;
	}

	ViInt64 first_element_samples;
	ViInt64 actual_elements_samples = 0;
	ViInt64 available_elements_samples = 0;

	int elements_to_acquire = (triggers_per_read * samples_per_trigger) / 2;
	
	do
	{
		auto rc = digitizer.stream_fetch_data(
			samples_channel.c_str(),
			elements_to_acquire,
			samples_buffer->get_size(),
			(ViInt32 *)samples_buffer->get_raw_unaquired(),
			&available_elements_samples, &actual_elements_samples, &first_element_samples);
		if (rc.second == Digitizer::Error)
		{
			throw std::runtime_error(rc.first);
		}
	} while (actual_elements_samples < elements_to_acquire);

	samples_buffer->advance_offset(first_element_samples);
	samples_buffer->advance_acquired(actual_elements_samples);

	return AcquiredData(stamps, samples_buffer, samples_per_trigger);
}
