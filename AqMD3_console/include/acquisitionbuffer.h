#ifndef ACQUISITION_BUFFER_H
#define ACQUISITION_BUFFER_H

#include <vector>

class AcquisitionBuffer {
private:
	int id;
	std::vector<int32_t> data;
	int capacity;
	int count;

public:
	AcquisitionBuffer(int id, size_t size) :
		id(id),
		data(size),
		capacity(size),
		count(0)
	{}

	int get_id();
	int space_available();
	int32_t* get_raw();
	int32_t* get_raw_at(int indx);
	void reduce_available(int num);
	void reset();
};

#endif // !ACQUISITION_BUFFER_H