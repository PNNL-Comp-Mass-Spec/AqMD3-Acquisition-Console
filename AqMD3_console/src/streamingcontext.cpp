#include "../include/streamingcontext.h"

#include <iostream>

static void check_error(ViStatus status) {

	ViInt32 ec;
	ViChar error_message[512];

	if (status > 0)
	{
		AqMD3_GetError(VI_NULL, &ec, sizeof(error_message), error_message);
		cout << "Error Code: " + std::to_string(ec) + "Error Message: " + error_message;

	}
	else if (status < 0)
	{
		AqMD3_GetError(VI_NULL, &ec, sizeof(error_message), error_message);
		cout << "Error Code: " + std::to_string(ec) + "Error Message: " + error_message;
	}
}

void StreamingContext::start()
{
	int rc = AqMD3_InitiateAcquisition(session);
	check_error(rc);
}

void StreamingContext::stop()
{
	int rc = AqMD3_Abort(session);
	check_error(rc);
}