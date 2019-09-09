#include "../include/streamingcontext.h"

#include <iostream>

void StreamingContext::start()
{
	int rc = AqMD3_InitiateAcquisition(session);
	std::cout << "init acq - rc: " << rc << std::endl;
}

void StreamingContext::stop()
{
	AqMD3_Abort(session);
}
