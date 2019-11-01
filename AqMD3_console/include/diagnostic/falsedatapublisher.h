#ifndef FALSE_DATA_PUBLISHER_H
#define FALSE_DATA_PUBLISHER_H

#include "../acquireddata.h"
#include "../acquisitioncontrol.h"
#include "../framepublisher.h"
#include "../acquireddata.h"
#include "../streamingcontext.h"

#include <vector>
#include <iostream>

using namespace std;

class FalseDataPublisher : public AcquisitionControl, public FramePublisher<AcquiredData> {
private:
	AcquiredData *data1;
	int count;
	bool should_exit;

public:
	FalseDataPublisher(int record_size, int count)
		: count(count)
		, should_exit(false)
	{
		auto gate_start = 600;
		auto gate_stop = 680;

		auto gate_start2 = 5200;
		auto gate_stop2 = 5266;

		vector<AcquiredData::TriggerData> stamps1;
		for (int i = 0; i < 100; i++)
		{
			AcquiredData::TriggerData td(0, 0.0, i);
			td.gate_data.emplace_back(gate_start, 0, gate_stop, 0, ((gate_stop - gate_start) * 8) / 2);
			td.gate_data.emplace_back(gate_start2, 0, gate_stop2, 0, ((gate_stop2 - gate_start2) * 8) / 2);
			stamps1.push_back(td);
		}

		//std::cout << "record samples: " << stamps1[0].gate_data[0].get_stop_sample_index() - stamps1[0].gate_data[0].get_start_sample_index() << std::endl;

		auto ac_buf1 = make_shared<AcquisitionBuffer>(0, record_size * 256);

		int offset = (((gate_stop - gate_start) * 8) / 2) + (((gate_stop2 - gate_start2) * 8) / 2);

		vector<int32_t> vec(offset);

		for (int i = 0; i < (((gate_stop - gate_start) * 8) / 2); i++)
		{
			vec[i] = 0x02220222;
		}

		for (int i = (((gate_stop - gate_start) * 8) / 2); i < offset; i++)
		{
			vec[i] = 0x01110111;
		}

		for (int trig = 0; trig < 100; trig++)
		{
			memcpy(&ac_buf1->get_raw_data()[trig * offset], vec.data(), offset * 4);
		}
		ac_buf1->advance_acquired(offset * 100);

		data1 = new AcquiredData(stamps1, ac_buf1, record_size);
	}

	void start() override;

	void stop() override;

	std::thread worker_handle;


};

void FalseDataPublisher::start()
{
	worker_handle = thread([&]()
	{
		for (int i = 0; i < count; i++)
		{
			std::cout << i << std::endl;
			notify(*data1, SubscriberType::BOTH);
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		}
		std::cout << "publisher exited" << std::endl;
	});
}

void FalseDataPublisher::stop()
{
	notify_completed_and_wait();
	worker_handle.join();
	std::cout << "-- stop called and completed --" << std::endl;
}

#endif // !FALSE_DATA_PUBLISHER_H