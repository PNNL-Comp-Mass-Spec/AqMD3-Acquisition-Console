#ifndef ACQUISITION_BUFFER_H
#define ACQUISITION_BUFFER_H

#include <vector>
#include <iostream>

class AcquisitionBuffer {
private:
	std::vector<int32_t> data;
	int acquired_index;
	int processed_index;
	int offset;

	uint64_t triggers_per_read;
	uint64_t samples_per_trigger;

public:
	AcquisitionBuffer(uint64_t triggers_per_read, uint64_t samples_per_trigger)
		: data(triggers_per_read * samples_per_trigger)
		, acquired_index(0)
		, processed_index(0)
		, offset(0)
		, triggers_per_read(triggers_per_read)
		, samples_per_trigger(samples_per_trigger)
	{}

	size_t get_size() { return samples_per_trigger * triggers_per_read; };
	int get_available();
	int get_unprocessed();
	int get_processed();
	int get_acquired();
	int32_t* get_raw_data();
	int32_t* get_raw_unprocessed();
	int32_t* get_raw_unaquired();
	void advance_offset(int num);
	void advance_processed(int num);
	void advance_acquired(int num);
	void reset();

	uint64_t get_triggers_per_read() { return triggers_per_read; }
	uint64_t get_samples_per_trigger() { return samples_per_trigger; }
};

#endif // !ACQUISITION_BUFFER_H