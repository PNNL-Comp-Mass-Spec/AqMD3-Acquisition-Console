#ifndef DIGITIZER_H
#define DIGITIZER_H

#include "streamingcontext.h"
#include "acquisitionbufferpool.h"
#include "acquireddata.h"
#include "AqMD3.h"
#include <tuple>
#include <memory>
#include <chrono>
#include <string>

class Digitizer {
private:
	std::string digitizer;
	std::string options;

public:
	std::unique_ptr<StreamingContext> configure_cst();
	std::unique_ptr<StreamingContext> configure_cst_zs1();

public:
	Digitizer() :
		digitizer("PXI3::0::0::INSTR"),
		options("Simulate=false, DriverSetup= Model=SA220P")
	{}
};

#endif // ! DIGITIZER_H
