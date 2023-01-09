#include "../include/libaqmd3/acquisitionbufferpool.h"

size_t AcquisitionBufferPool::get_available_buffers()
{
	size_t count = buffer_max_count;
	for (const auto& buf : samples_buffer_pool)
	{
		if (buf.use_count() >= 2)
			--count;
	}

	return count;
}

std::shared_ptr<AcquisitionBuffer> AcquisitionBufferPool::get_buffer()
{
	int total = samples_buffer_pool.size();
	for (int i = 0; i < total; i++)
	{
		int index = (buffer_takes + i) % total;
		//TODO .use_count() not reliable, find other way
		if (samples_buffer_pool[index].use_count() == 1)	
		{
			buffer_takes++;
			samples_buffer_pool[index]->reset();
			return samples_buffer_pool[index];
		}
	}

	samples_buffer_pool.push_back(std::make_shared<AcquisitionBuffer>(triggers_per_read, samples_per_trigger));
	buffer_takes++;
	return samples_buffer_pool[total];
}