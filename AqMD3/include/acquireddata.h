#ifndef ACQUIRED_DATA_H
#define ACQUIRED_DATA_H

#include "../include/acquisitionbufferpool.h"
#include "../include/acquisitionbuffer.h"

#include <tuple>
#include <vector>

class AcquiredData {
private:
	AcquisitionBufferPool *data_repo;
	AcquisitionBufferPool *markers_repo;

	std::vector<AcquisitionBuffer*> ordered_markers_cache;
	std::vector<AcquisitionBuffer*> ordered_data_cache;

	int triggers;

	const int trigger_offset_block_size = 16;
	const int gate_offset_block_size = 2;

public:
	AcquiredData(AcquisitionBufferPool* data, AcquisitionBufferPool* markers, int triggers) :
		data_repo(data),
		markers_repo(markers),
		triggers(triggers)
	{}

	void add_markers_buffer(AcquisitionBuffer *buffer);
	void add_data_buffer(AcquisitionBuffer *buffer);

	void process(/* data sink */);
};

#endif // !ACQUIRED_DATA_H
