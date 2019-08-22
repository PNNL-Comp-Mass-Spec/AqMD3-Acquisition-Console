#include "../include/acquisitionbuffer.h"

int AcquisitionBuffer::get_id()
{
	return id;
}

int AcquisitionBuffer::space_available()
{
	return capacity - count;
}

int32_t* AcquisitionBuffer::get_raw()
{
	return &data[count];
}

int32_t* AcquisitionBuffer::get_raw_at(int indx)
{
	return &data[count];
}

void AcquisitionBuffer::reduce_available(int num)
{
	capacity += num;
}

void AcquisitionBuffer::reset()
{
	count = 0;
}