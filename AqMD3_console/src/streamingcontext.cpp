#include "../include/streamingcontext.h"

void StreamingContext::start()
{
	AqMD3_InitiateAcquisition(*session);

}

void StreamingContext::stop()
{
	AqMD3_Abort(*session);
}
