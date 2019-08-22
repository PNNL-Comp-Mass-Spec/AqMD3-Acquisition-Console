#ifndef ACQUISITION_BUFFER_POOL_H
#define ACQUISITION_BUFFER_POOL_H

#include "acquisitionbuffer.h"
#include <deque>
#include <map>

using namespace std;

class AcquisitionBufferPool {
private:
	deque<AcquisitionBuffer*> available;
	map<int, AcquisitionBuffer*> in_use;

public:
	AcquisitionBufferPool(size_t buf_size, int count) :
		available(count)
	{
		for (int i = 0; i < count; i++)
			available.push_front(new AcquisitionBuffer(i, buf_size));
	}
	~AcquisitionBufferPool()
	{
		for (auto& buf : available)
			delete buf;
		for (auto& kvBuf : in_use)
			delete kvBuf.second;
	}

	AcquisitionBuffer* next_available();
	void return_in_use(AcquisitionBuffer* buffer);
	int get_available();
	int get_in_use();
};

#endif // !ACQUISITION_BUFFER_POOL_H
