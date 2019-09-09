#ifndef ACQUIRED_DATA_H
#define ACQUIRED_DATA_H

#include "acquisitionbufferpool.h"
#include "acquisitionbuffer.h"

#define NOMINMAX 
#undef min
#undef max
#include "message.pb.h"

#include <tuple>
#include <vector>
#include <zmq.hpp>

//using namespace zmq;

class AcquiredData {
private:
	AcquisitionBufferPool *sample_repo;
	//AcquisitionBufferPool *markers_repo;
	//AcquisitionBuffer* markers_cache;
	AcquisitionBuffer* sample_buffer;

	vector<uint64_t> stamps;

	const int trigger_offset_block_size = 16;
	const int gate_offset_block_size = 2;

public:
	AcquiredData(vector<uint64_t> stamps, AcquisitionBufferPool* sample_pool, AcquisitionBuffer* sample_buffer)
		: sample_repo(sample_pool), sample_buffer(sample_buffer), stamps(stamps)
	{}

	//AcquiredData(AcquisitionBufferPool* data, AcquisitionBufferPool* markers,
	//	AcquisitionBuffer* markers_cache, AcquisitionBuffer* data_cache) :
	//	data_repo(data),
	//	markers_repo(markers),
	//	markers_cache(markers_cache),
	//	data_cache(data_cache)
	//{}

	//std::tuple<std::vector<int32_t>, std::vector<uint32_t>, std::vector<uint64_t>> process();

	void process(zmq::socket_t& pub_soc, Message& msg);
};

#endif // !ACQUIRED_DATA_H
