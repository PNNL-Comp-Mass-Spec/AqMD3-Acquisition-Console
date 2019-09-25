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

class DataSegment {
public:
	uint64_t first_index;
	std::vector<int32_t> peak_data;

	DataSegment(const uint64_t first_index, const std::vector<int32_t> peak_data)
		: first_index(first_index)
		, peak_data(peak_data)
	{}
};

class TriggerData {
public:
	uint64_t timestamp;
	double subsample_pos;
	uint32_t index;
	// start, end, diff
	std::vector<std::tuple<uint64_t, uint64_t, uint64_t, uint64_t>> gate_cage;

	std::vector<DataSegment> segments;

	TriggerData(uint64_t timestamp, uint32_t index, double subsample_pos)
		: timestamp(timestamp)
		, index(index)
		, subsample_pos(subsample_pos)
		, gate_cage()
		, segments()
	{}
};

class AcquiredData {
private:
public:
	AcquisitionBufferPool *sample_repo;
	//AcquisitionBufferPool *markers_repo;
	//AcquisitionBuffer* markers_cache;
	AcquisitionBuffer* sample_buffer;

	const int trigger_offset_block_size = 16;
	const int gate_offset_block_size = 2;
	vector<TriggerData> stamps;

public:
	AcquiredData(vector<TriggerData> stamps, AcquisitionBufferPool* sample_pool, AcquisitionBuffer* sample_buffer)
		: sample_repo(sample_pool), sample_buffer(sample_buffer), stamps(stamps)
	{}

	AcquiredData()
	{}

	//AcquiredData(AcquisitionBufferPool* data, AcquisitionBufferPool* markers,
	//	AcquisitionBuffer* markers_cache, AcquisitionBuffer* data_cache) :
	//	data_repo(data),
	//	markers_repo(markers),
	//	markers_cache(markers_cache),
	//	data_cache(data_cache)
	//{}

	//std::tuple<std::vector<int32_t>, std::vector<uint32_t>, std::vector<uint64_t>> process();

	const int32_t& operator[](const size_t val) {
		return sample_buffer->get_raw_unprocessed()[val];
	}

	std::vector<TriggerData> process();
	void process(zmq::socket_t& pub_soc, Message& msg);
};

#endif // !ACQUIRED_DATA_H
