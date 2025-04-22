#include "../include/libaqmd3/streamingcontext.h"
#include "../include/libaqmd3/digitizer.h"
#include "../include/libaqmd3/helpers.h"

#include <iostream>
#include <stdexcept>

void StreamingContext::start()
{
	check_and_throw_on_error(digitizer.begin_acquisition());
}

void StreamingContext::stop()
{
	check_and_throw_on_error(digitizer.abort_acquisition());
}

bool StreamingContext::get_is_acquiring()
{
	auto result = digitizer.get_is_measuring();
	check_and_throw_on_error(result);
	auto status = std::get<2>(result);

	return status == AQMD3_VAL_ACQUISITION_STATUS_RESULT_TRUE;
}

