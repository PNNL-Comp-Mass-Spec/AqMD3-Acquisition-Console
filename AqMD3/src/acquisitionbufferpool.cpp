#include "../include/acquisitionbufferpool.h"

AcquisitionBuffer* AcquisitionBufferPool::next_available()
{
	if (available.size() == 0)
		return nullptr; //throw exception
	
	AcquisitionBuffer* buf = available.front();
	available.pop_front();

	buf->reset();
	in_use.insert(std::pair<int, AcquisitionBuffer*>(buf->get_id(), buf));

	return buf;
}

void AcquisitionBufferPool::return_in_use(AcquisitionBuffer* buffer)
{
	in_use.erase(buffer->get_id());
	available.push_front(buffer);
}

int AcquisitionBufferPool::get_available()
{
	return available.size();
}

int AcquisitionBufferPool::get_in_use()
{
	return in_use.size();
}
