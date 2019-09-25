#ifndef SA220_H
#define SA220_H

#include "digitizer.h"
#include <string>

class SA220 : public Digitizer {
public:
	std::string const samples_channel_1;
	std::string const markers_channel_1;
	std::string const samples_channel_2;
	std::string const markers_channel_2;

	std::string const visa_device_id;
	std::string const options;

private:
	uint32_t record_size;

public:
	SA220(std::string device, std::string options) : Digitizer(device, options)
		, samples_channel_1("StreamCh1")
		, samples_channel_2("StreamCh2")
		, markers_channel_1("MarkersCh1")
		, markers_channel_2("MarkersCh2")
	{}

};

#endif // !SA220P
