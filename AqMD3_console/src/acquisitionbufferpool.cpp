#include "../include/acquisitionbufferpool.h"

#include <exception>
#include <iostream>

AcquisitionBuffer* AcquisitionBufferPool::next_available()
{
	//if (available.size() == 0) {} //throw exception
	try
	{
		AcquisitionBuffer* buf = available.front();
		available.pop_front();

		buf->reset();
		in_use.insert(std::pair<int, AcquisitionBuffer*>(buf->get_id(), buf));

		return buf;
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return nullptr;
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