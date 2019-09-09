#ifndef DGT_CONTEXT_H
#define DGT_CONTEXT_H

#include "acquisitioncontext.h"
#include "AqMD3.h"

#include <vector>

//class DgtContext : public AcquisitionContext {
//private:
//	std::string channel;
//	//AcquisitionBuffer buffer;
//
//public:
//	DgtContext(ViSession session, /*AcquisitionBuffer& buffer,*/ std::string channel) : 
//		AcquisitionContext(session),
//		channel(channel)
//		//buffer(buffer)
//	{}
//
//		//std::unique_ptr<AcquiredData> acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) override;
//		std::tuple<std::vector<ViInt16>, std::vector<ViInt64>, std::vector<ViReal64>> acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) override;
//};

#endif