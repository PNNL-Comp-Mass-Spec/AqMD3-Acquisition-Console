#include "../include/acquisitionbuffer.h"

int AcquisitionBuffer::get_size()
{
	return data.size();
}

int AcquisitionBuffer::get_id()
{
	return id;
}

int AcquisitionBuffer::get_available()
{
	return data.size() - acquired_index - offset;
}

int AcquisitionBuffer::get_unprocessed()
{
	return acquired_index - processed_index;
}

int AcquisitionBuffer::get_processed()
{
	return processed_index;
}

int AcquisitionBuffer::get_acquired()
{
	return acquired_index;
}

int32_t* AcquisitionBuffer::get_raw_data()
{
	return data.data();
}

int32_t* AcquisitionBuffer::get_raw_unprocessed()
{
	return &data[processed_index + offset];
}

int32_t* AcquisitionBuffer::get_raw_unaquired()
{
	return &data[acquired_index + offset];
}

void AcquisitionBuffer::advance_processed(int num)
{
	processed_index += num;
}

void AcquisitionBuffer::advance_acquired(int num)
{
	acquired_index += num;
}

void AcquisitionBuffer::advance_offset(int num)
{
	offset += num;
}

void AcquisitionBuffer::reset()
{
	processed_index = 0;
	acquired_index = 0;
	offset = 0;
}

void AcquisitionBuffer::reset_processed() 
{
	processed_index = 0;
	acquired_index = 0;
}

std::vector<int32_t>::const_iterator AcquisitionBuffer::begin()
{
	return data.begin() + offset;
}
