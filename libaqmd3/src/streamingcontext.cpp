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

