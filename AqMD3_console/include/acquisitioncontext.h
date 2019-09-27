#ifndef ACQUISITION_CONTEXT_H
#define ACQUISITION_CONTEXT_H

#include <tuple>
#include <vector>
#include <memory>
#include <chrono>
#include "AqMD3.h"

class AcquisitionContext {
protected:
	ViSession session;

public:
	AcquisitionContext(ViSession session) : session(session)
	{};

	//virtual std::unique_ptr<AcquiredData> acquire(int32_t triggers, std::chrono::milliseconds timeoutMs) = 0;
};

#endif