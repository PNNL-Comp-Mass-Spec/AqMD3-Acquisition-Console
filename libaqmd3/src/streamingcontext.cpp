#include "../include/libaqmd3/streamingcontext.h"
#include "../include/libaqmd3/digitizer.h"
#include <iostream>
#include <stdexcept>

void StreamingContext::start()
{
	auto result = digitizer.begin_acquisition();
	if (result.second == Digitizer::None)
	{
		throw std::runtime_error(result.first);
	}
}

void StreamingContext::stop()
{
	auto result = digitizer.abort_acquisition();
	if (result.second == Digitizer::None)
	{
		throw std::runtime_error(result.first);
	}
}

int StreamingContext::get_available_buffers()
{
	int count = buffer_max_count;
	for (const auto& buf : samples_buffer_pool)
	{
		if (buf.use_count() >= 2)
			--count;
	}

	return count;
}

std::shared_ptr<AcquisitionBuffer> StreamingContext::get_buffer()
{
	int total = samples_buffer_pool.size();
	for (int i = 0; i < total; i++)
	{
		int index = (buffer_takes + i) % total;
		if (samples_buffer_pool[index].use_count() == 1)	// .use_count() not reliable
		{
			buffer_takes++;
			samples_buffer_pool[index]->reset();
			return samples_buffer_pool[index];
		}
	}

	if (total == buffer_max_count)
	{
		throw std::runtime_error("buffer max count reached");
	}

	samples_buffer_pool.push_back(std::make_shared<AcquisitionBuffer>(total, samples_buffer_size));
	buffer_takes++;
	return samples_buffer_pool[total];
}
