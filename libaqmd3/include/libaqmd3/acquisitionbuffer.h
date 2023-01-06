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

public:
	AcquisitionBuffer(size_t size) 
		: data(size)
		, acquired_index(0)
		, processed_index(0)
		, offset(0)
	{}

	int get_size();
	int get_id();
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
};

#endif // !ACQUISITION_BUFFER_H