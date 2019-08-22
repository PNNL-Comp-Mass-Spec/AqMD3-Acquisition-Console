#ifndef DIGITIZER_H
#define DIGITIZER_H

#include "streamingcontext.h"
#include "acquisitionbufferpool.h"
#include "acquireddata.h"
#include "AqMD3.h"
#include <tuple>
#include <memory>
#include <chrono>

class Digitizer {
private:
	ViChar *digitizer;
	ViChar *options;

public:
	Digitizer() :
		digitizer("PXI3::0::0::INSTR"),
		options("Simulate=false, DriverSetup= Model=SA220P")
	{}

	ViSession& configure();
};

#endif // ! DIGITIZER_H
