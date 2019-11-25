#include "../include/streamingcontext.h"

#include <iostream>

static void check_error(ViStatus status) {

	ViInt32 ec;
	ViChar error_message[512];

	if (status > 0)
	{
		AqMD3_GetError(VI_NULL, &ec, sizeof(error_message), error_message);
		std::cout << "Error Code: " + std::to_string(ec) + "Error Message: " + error_message;

	}
	else if (status < 0)
	{
		AqMD3_GetError(VI_NULL, &ec, sizeof(error_message), error_message);
		std::cout << "Error Code: " + std::to_string(ec) + "Error Message: " + error_message;
	}
}

void StreamingContext::start()
{
	should_stop = false;
	int rc = AqMD3_InitiateAcquisition(session);
	check_error(rc);
}

void StreamingContext::stop()
{
	should_stop = true;
	int rc = AqMD3_Abort(session);
	check_error(rc);
}

std::shared_ptr<AcquisitionBuffer> StreamingContext::get_buffer()
{
	int total = samples_buffer_pool.size();
	for (int i = 0; i < total; i++)
	{
		int index = (buffer_takes + i) % total;
		if (samples_buffer_pool[index].unique())
		{
			buffer_takes++;
			samples_buffer_pool[index]->reset();
			return samples_buffer_pool[index];
		}
	}

	if (total == buffer_max_count)
	{
		throw std::string("buffer max count reached");
	}

	samples_buffer_pool.push_back(std::make_shared<AcquisitionBuffer>(total, samples_buffer_size));
	buffer_takes++;
	return samples_buffer_pool[total];
}
