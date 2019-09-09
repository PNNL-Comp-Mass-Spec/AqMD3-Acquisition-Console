#ifndef DIGITIZER_H
#define DIGITIZER_H

#include "streamingcontext.h"
#include "acquisitionbufferpool.h"
#include "acquireddata.h"
#include "acquisitioncontext.h"
#include "AqMD3.h"
#include <tuple>
#include <memory>
#include <chrono>
#include <string>

class Digitizer {
private:
	std::string resource;
	std::string options;
	ViSession session;

public:
	//std::unique_ptr<StreamingContext> configure_cst();
	std::shared_ptr<StreamingContext> configure_cst_zs1();
	//std::unique_ptr<AcquisitionContext> configure_dgt();
	
public:
	Digitizer(std::string resource, std::string options) :
		session(VI_NULL),
		resource(resource), //resource("PXI3::0::0::INSTR"),
		options(options) //options("Simulate=false, DriverSetup= Model=SA220P")
	{
		AqMD3_InitWithOptions((ViChar *)resource.c_str(), VI_FALSE, VI_FALSE, options.c_str(), &session);
	}
	~Digitizer()
	{
		AqMD3_close(session);
	}
};

#endif // ! DIGITIZER_H
