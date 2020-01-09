#ifndef TIME_HELPERS_H
#define TIME_HELPERS_H

#include <Windows.h>
#include <string>

static std::string timestamp_now() 
{
	SYSTEMTIME time;
	char currentTime[256] = { 0 };

	GetSystemTime(&time);
	std::sprintf(currentTime, "%i:%i:%i.%i", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

	return currentTime;
}

#endif