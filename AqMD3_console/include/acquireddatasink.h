#ifndef ACQUIRED_DATA_SINK_H
#define ACQUIRED_DATA_SINK_H

#include "UIMFWriter/encodedresult.h"
#include <vector>

class AcquiredDataSink {
	virtual void process(std::vector<EncodedResult> elements) = 0;
};

#endif // !ACQUIRED_DATA_SINK_H
