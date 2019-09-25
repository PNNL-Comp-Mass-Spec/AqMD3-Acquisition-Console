#ifndef ACQUISITION_BUFFER_H
#define ACQUISITION_BUFFER_H

#include <vector>
#include <iostream>

class AcquisitionBuffer {
private:
	std::vector<int32_t> data;
	int id;
	int acquired_index;
	int processed_index;
	int offset;

public:
	AcquisitionBuffer(int id, size_t size) :
		id(id),
		data(size),
		acquired_index(0),
		processed_index(0),
		offset(0)
	{
		//uint64_t const alignmentMask = uint64_t(64 - 1);
		//for (size_t i = 0; i < 16; ++i)
		//{
		//	uint64_t const ptr = reinterpret_cast<uint64_t>(&data[i]);
		//	if ((ptr & alignmentMask) == 0)
		//	{
		//		// std::cout << "offset" << offset << std::endl;
		//		offset = i;
		//		break;
		//	}
		//}
	}

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
	void reset_processed();
	std::vector<int32_t>::const_iterator begin();
};

#endif // !ACQUISITION_BUFFER_H