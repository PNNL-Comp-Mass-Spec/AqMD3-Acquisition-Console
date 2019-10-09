// FalkorAcquisition.cpp : Defines the entry point for the application.

#include "../include/acquisitionbufferpool.h"
#include "../include/server.h"
#include "../include/digitizer.h"
#include "../include/acquisitioncontext.h"
#include "../include/dgtcontext.h"
#include "../include/acquireddata.h"
#include "../include/datapublisher.h"
#include "../include/sa220.h"

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

#include <UIMFWriter/uimfwriter.h>

#include "AqMD3.h"

#include <zmq.hpp>
#include <picosha2.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <numeric>
#include <random>
#include <algorithm>
#include <iterator>
#include <functional>
#include <mutex>
#include <tuple>
#include <string>

#include <queue>
#include <fstream>

#include <cstring>
#include <cmath>
#include <ctime>

#include <visa.h>

using namespace std;

//static void digitizer_worker(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, unique_ptr<StreamingContext> context);
//static void publish_worker(zmq::socket_t &pusher, std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, ofstream& readable_file,
//	UimfWriter& uimfwriter);

static void digitizer_worker(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, unique_ptr<StreamingContext> context);
static void publish_worker(std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, DataPublisher publisher);

static std::pair<uint64_t, double> get_tof_width(SA220& digitizer);

bool should_exit = false;

int main(int argc, char *argv[]) {
	if (argc != 3)
		return 0;

	int triggers = atoi(argv[1]);
	std::cout << "triggers: " << triggers << endl;
	std::string uimf_file = argv[2];
	 
	UimfWriter writer(uimf_file);

	auto server = new Server("tcp://*:5555");
	SA220 digitizer("PXI3::0::0::INSTR", "Simulate=false, DriverSetup= Model=SA220P");

	digitizer.set_sampling_rate(1000000000.0);
	digitizer.set_trigger_parameters(digitizer.trigger_external, 0.5, true);
	digitizer.set_channel_parameters(digitizer.channel_1, digitizer.full_scale_range_500mv, 0.0);

	vector<thread> threads;
	should_exit = false;
	condition_variable signal;
	queue<AcquiredData> dataQueue;
	mutex lock;

	//auto t = std::chrono::system_clock::now();
	//auto tp = std::chrono::system_clock::to_time_t(t);
	//string tps = std::ctime(&tp);

	//char buffer[512];
	//std::string output_template = "output_%d_threshold_200_hysteresis_%d_presamples.csv";

	//for (int presamples = 8; presamples <= 8; presamples += 8)
	//{
	//	for (int threshold = 0; threshold <= 0; threshold += 500)
	//	{
	//		should_exit = false;
	//		condition_variable signal;
	//		queue<AcquiredData> dataQueue;
	//		mutex lock;

	//		vector<thread> threads;
	//		ofstream readable_file;
	//		sprintf(buffer, output_template.c_str(), threshold, presamples);
	//		std::string out_file = buffer;
	//		readable_file.open(out_file);

	//		auto dig = digitizer.configure_cst_zs1(digitizer.channel_1, triggers, Digitizer::ZeroSuppressParameters(threshold, 200, presamples, 0));

	//		threads.push_back(thread(publish_worker, ref(publisher), ref(signal), ref(lock), ref(dataQueue), ref(readable_file), ref(writer)));
	//		threads.push_back(thread(digitizer_worker, ref(signal), ref(dataQueue), std::move(dig)));

	//		for (auto& thread : threads)
	//		{
	//			thread.join();
	//		}
	//		readable_file.close();
	//	}
	//}

	///* server stuff */
	server->register_handler([&](Server::ReceivedRequest req) 
	{
		for (auto& payload = req.payload.begin(); payload != req.payload.end(); payload++)
		{
			if (*payload == "num instruments")
			{
				req.send_response("1");
				return;
			}

			if (*payload == "info")
			{
				continue;
			}

			if (*payload == "init")
			{
				continue;
			}

			if (*payload == "horizontal")
			{
				continue;
			}

			if (*payload == "vertical")
			{
				continue;
			}

			if (*payload == "trig class")
			{
				continue;
			}

			if (*payload == "trig source")
			{
				continue;
			}

			if (*payload == "mode")
			{
				continue;
			}

			if (*payload == "config digitizer")
			{
				continue;
			}

			if (*payload == "post samples")
			{
				continue;
			}

			if (*payload == "pre samples")
			{
				continue;
			}

			if (*payload == "setup array")
			{
				continue;
			}

			if (*payload == "acquire frame")
			{
				continue;
			}

			if (*payload == "acquire")
			{
				std::cout << "rec'd \'acquire\'" << endl;
				std::pair<uint64_t, double> tof_params = get_tof_width(digitizer);
				std::cout << "samples per trigger: " << get<0>(tof_params) << std::endl;
				digitizer.set_record_size(get<0>(tof_params));

				auto context = digitizer.configure_cst_zs1(digitizer.channel_1, triggers, Digitizer::ZeroSuppressParameters(0, 200, 8, 0));


				threads.push_back(thread(publish_worker, ref(signal), ref(lock), ref(dataQueue), DataPublisher(server->get_publisher("tcp://*:6546"), get<0>(tof_params))));
				threads.push_back(thread(digitizer_worker, ref(signal), ref(dataQueue), std::move(context)));
			
				vector<string> to_send(2);

				TofWidthMessage tofMsg;
				tofMsg.set_num_samples(std::get<0>(tof_params));
				tofMsg.set_pusher_pulse_width(std::get<1>(tof_params));

				to_send[0] = (tofMsg.SerializeAsString());

				vector<uint8_t> hash(picosha2::k_digest_size);
				picosha2::hash256(to_send[0].begin(), to_send[0].end(), hash.begin(), hash.end());

				to_send[1] = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
				req.send_responses(to_send);

				return;
			}

			if (*payload == "tof width")
			{

				std::pair<uint64_t, double> tof_params = get_tof_width(digitizer);

				vector<string> to_send(2);

				TofWidthMessage tofMsg;
				tofMsg.set_num_samples(std::get<0>(tof_params));
				tofMsg.set_pusher_pulse_width(std::get<1>(tof_params));

				to_send[0] = (tofMsg.SerializeAsString());

				vector<uint8_t> hash(picosha2::k_digest_size);
				picosha2::hash256(to_send[0].begin(), to_send[0].end(), hash.begin(), hash.end());

				to_send[1] = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
				req.send_responses(to_send);
				
				return;
			}

			if (*payload == "stop")
			{
				continue;
			}

			if (*payload == "invert")
			{
				continue;
			}

			if (*payload == "reset timestamps")
			{
				continue;
			}

			if (*payload == "enable io port")
			{
				continue;
			}

			if (*payload == "disable io port")
			{
				continue;
			}
		}
	});

	thread t([&] { server->run(); });

	getchar();
	should_exit = true;
	cout << "stopping -- press again to stop" << endl;

	server->stop();
	t.join();

	getchar();
	return 0;
}

static void digitizer_worker(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, unique_ptr<StreamingContext> context)
{
	std::cout << "STARTING ACQUISITION" << endl;
	uint64_t total_acquire_time = 0;
	context->start();
	while(!should_exit)
	{
		auto data = context->acquire(std::chrono::milliseconds(100));
		resultsQueue.push(data);
		sig.notify_one();
	}
	context->stop();
}

static void publish_worker(std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, DataPublisher publisher)
{
	int process_index = 0;
	vector<AcquiredData::TriggerData> vec;
	uint64_t average_processing_time = 0;

	while (!should_exit)
	{
		{
			std::unique_lock<std::mutex> lock(lockable);
			sig.wait(lock);
		}

		while (!workQueue.empty())
		{
			AcquiredData ae = workQueue.front();
			workQueue.pop();
			auto result = ae.process(1, 100 * process_index++);
			publisher.process(result);
		}
	}
}

std::pair<uint64_t, double> get_tof_width(SA220& digitizer)
{
	digitizer.set_record_size(1024);
	auto dig_context = digitizer.configure_cst(digitizer.channel_1, 100);

	dig_context->start();
	AcquiredData result = dig_context->acquire(std::chrono::milliseconds(80));
	dig_context->stop();

	uint64_t total = 0;

	for (int i = 0; i < result.stamps.size() - 1; i++)
	{
		total += result.stamps[i + 1].timestamp - result.stamps[i].timestamp;
	}

	total = ((total / (result.stamps.size() - 1)) / 2);
	int r = total % 16;
	total = total - r - /* 2 * */ 128;
	double time = total / /*(2 * */ pow(10, 9) /* ) */;

	return std::make_pair(total, time);
}

//static void publish_worker(zmq::socket_t &pusher, std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, ofstream& readable_file, UimfWriter& uimfwriter)
//{
//	int process_index = 0;
//	vector<AcquiredData::TriggerData> vec;
//	//ofstream datasink;
//	//datasink.open("nul");
//
//	//vector<vector<int32_t>> vecs;
//	//for (int i = 0; i < 10; i++)
//	//{
//	//	vecs.emplace_back(record_size);
//	//}
//	uint64_t total_processing_time = 0;
//	UimfFrame frame(0, 94016, 1, 10000, 1, 0, "");
//	while (!should_exit)
//	{
//		{
//			std::unique_lock<std::mutex> lock(lockable);
//			sig.wait(lock);
//		}
//
//		while (!workQueue.empty())
//		{
//			Message msg;
//			msg.mutable_tic()->Reserve(256);
//			msg.mutable_time_stamps()->Reserve(256);
//
//			auto t1 = chrono::high_resolution_clock::now();
//			AcquiredData ae = workQueue.front();
//			workQueue.pop();
//
//			auto result = ae.process(1, 100 * process_index++);
//			//std::cout << "RESULTS COUNT: " << result.size() << endl;
//			//frame.append_encoded_results(result);
//			auto t2 = chrono::high_resolution_clock::now();
//			
//			//for (int i = 0; i < 100; i++)
//			//{
//			//	auto obj = result[i];
//			//	int seg_index = 0;
//			//	std::cout << "\t---------LARGEST ITEM INDEX: " << obj.index_max_intensity << endl;
//			//	std::cout << "\t---------LARGEST ITEM: " << obj.bpi << endl;
//			//	for (auto val : obj.encoded_spectra)
//			//	{
//			//		if (val < 0)
//			//		{
//			//			std::cout << "Zero Val: " << val << endl;
//			//			seg_index += (-1 * val);
//			//			continue;
//			//		}
//
//			//		vecs[i][seg_index++] = val;
//			//	}
//			//	std::cout << std::endl;
//			//}
//			total_processing_time += (t2 - t1).count();
//			if ((process_index % 100) == 0)
//				cout << "processing -- " << process_index << endl;
//			//std::cout << "ACQUISITION: " << process_index// << endl;
//			//	<<" PROCESSING TIME:" << (t2 - t1).count() << endl;
//		}
//	}
//
//	//readable_file << "index,trig_1,trig_2,trig_3,trig_4,trig_5,trig_6,trig_7,trig_8,trig_9,trig_10\n";
//	//for (int i = 0; i < record_size; i++)
//	//{
//	//	readable_file << i << ",";
//	//	for (int j = 0; j < vecs.size(); j++)
//	//	{
//	//		readable_file << vecs[j][i] << ",";
//	//	}
//	//	readable_file << std::endl;
//	//}
//	auto t1 = chrono::high_resolution_clock::now();
//	//uimfwriter.write_frame(frame);
//	auto t2 = chrono::high_resolution_clock::now();
//	//std::cout << "TOTAL WRITING TO UIMF TIME: " << (t2 - t1).count() << endl;
//	//std::cout << "TOTAL PROCESSING TIME: " << total_processing_time << endl;
//}