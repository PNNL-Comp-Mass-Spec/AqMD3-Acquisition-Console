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

#include <snappy.h>
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

static void digitizer_worker(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, unique_ptr<StreamingContext> context);
static void publish_worker(std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, DataPublisher publisher);
static void digitizer_worker_frame(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, unique_ptr<StreamingContext> context, UimfFrame uimf);
static void publish_worker_frame(std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, UimfFrame uimf);

static std::pair<uint64_t, double> get_tof_width(SA220& digitizer);

bool should_exit = false;
char ack[] = "ack";

int main(int argc, char *argv[]) {
	//if (argc != 3)
	//	return 0;

	//int triggers = atoi(argv[1]);
	//std::string uimf_file = argv[2];
	 
	//UimfWriter writer(uimf_file);

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

	///* server stuff */
	server->register_handler([&](Server::ReceivedRequest req) 
	{
		std::string command = req.payload[0];

		if (command == "num instruments")
		{
			ViSession rm = VI_NULL;
			viOpenDefaultRM(&rm);
			ViChar search[] = "PXI?*::INSTR";
			ViFindList find = VI_NULL;
			ViUInt32 count = 0;
			ViChar rsrc[256];

			ViStatus status = viFindRsrc(rm, search, &find, &count, rsrc);
			viClose(rm);

			req.send_response(std::to_string(count));
			return;
		}

		if (command == "info")
		{
			// TODO info
			req.send_response("");
			return;
		}

		if (command == "init")
		{
			// TODO init
			req.send_response(ack);
			return;
		}

		if (command == "horizontal")
		{
			if (req.payload.size() == 2)
			{
				auto horizontal_resolution = std::stod(req.payload[1]);
				digitizer.set_sampling_rate(horizontal_resolution);
			}
			
			req.send_response(ack);
			return;
		}

		if (command == "vertical")
		{
			if (req.payload.size() == 2)
			{
				auto offset_v = std::stod(req.payload[1]);
				digitizer.set_channel_parameters(digitizer.channel_1, digitizer.full_scale_range_500mv, offset_v);
			}

			req.send_response(ack);
			return;
		}

		if (command == "trig class")
		{
			// TODO trig class
			return;
		}

		if (command == "trig source")
		{
			// TODO trig source
			return;
		}

		if (command == "mode")
		{
			// TODO mode
			return;
		}

		if (command == "config digitizer")
		{
			// TODO config digitizer
			return;
		}

		if (command == "post samples")
		{
			// TODO post samples
			return;
		}

		if (command == "pre samples")
		{
			// TODO pre samples
			return;
		}

		if (command == "setup array")
		{
			// TODO setup array
			req.send_response(ack);
			return;
		}

		if (command == "acquire frame")
		{
			if (req.payload.size() == 2)
			{
				std::string uimf_req_msg; 
				snappy::Uncompress(req.payload[1].data(), req.payload[1].size(), &uimf_req_msg);
				auto uimf = UimfRequestMessage();
				uimf.MergeFromString(uimf_req_msg);

				std::pair<uint64_t, double> tof_params = get_tof_width(digitizer);
				std::cout << "samples per trigger: " << get<0>(tof_params) << std::endl;
				digitizer.set_record_size(get<0>(tof_params));

				UimfFrame frame(
					uimf.start_trigger(),
					get<0>(tof_params),
					uimf.nbr_accumulations(),
					uimf.frame_length(),
					uimf.frame_number(),
					uimf.offset_bins(),
					uimf.file_name()
				);

				auto context = digitizer.configure_cst_zs1(digitizer.channel_1, 100, Digitizer::ZeroSuppressParameters(0, 200, 8, 0));

				threads.push_back(thread(publish_worker_frame, ref(signal), ref(lock), ref(dataQueue), frame));
				threads.push_back(thread(digitizer_worker_frame, ref(signal), ref(dataQueue), std::move(context), frame));
			}

			req.send_response(ack);
			return;
		}

		if (command == "acquire")
		{
			std::pair<uint64_t, double> tof_params = get_tof_width(digitizer);
			std::cout << "samples per trigger: " << get<0>(tof_params) << std::endl;
			digitizer.set_record_size(get<0>(tof_params));

			auto context = digitizer.configure_cst_zs1(digitizer.channel_1, 100, Digitizer::ZeroSuppressParameters(0, 200, 8, 0));
			cout << "record size " << get<0>(tof_params) << endl;

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

		if (command == "tof width")
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

		if (command == "stop")
		{
			// TODO stop
			return;
		}

		if (command == "invert")
		{
			if (req.payload.size() == 2)
			{
				bool invert = req.payload[1] == "true";
				digitizer.set_channel_data_inversion(digitizer.channel_1, invert);
			}
			req.send_response(ack);
			return;
		}

		if (command == "reset timestamps")
		{
			return;
		}

		if (command == "enable io port")
		{
			if (req.payload.size() == 2)
			{
				auto val = std::stoi(req.payload[1]);
				if (val != 2)
					std::cerr << "wrong" << endl;

				digitizer.enable_io_port();
			}
			req.send_response(ack);
			return;
		}

		if (command == "disable io port")
		{
			if (req.payload.size() == 2)
			{
				auto val = std::stoi(req.payload[1]);
				if (val != 2)
					std::cerr << "wrong" << endl;

				digitizer.disable_io_port();
			}
			return;
		}
	});

	thread t([&] { server->run(); });

	getchar();
	should_exit = true;
	std::cout << "stopping -- press again to stop" << endl;

	server->stop();
	t.join();

	getchar();
	return 0;
}

static void digitizer_worker(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, unique_ptr<StreamingContext> context)
{
	//std::cout << "STARTING ACQUISITION" << endl;
	//context->start();
	//while(!should_exit)
	//{
	//	auto data = context->acquire(std::chrono::milliseconds(100));
	//	resultsQueue.push(data);
	//	sig.notify_one();
	//}
	//context->stop();
}

static void digitizer_worker_frame(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, unique_ptr<StreamingContext> context, UimfFrame frame)
{
	//std::cout << "STARTING ACQUISITION" << endl;
	//uint64_t total_acquire_time = 0;
	//context->start();
	//for(int i = 0; i < frame.frame_length; i += 100)
	//{
	//	auto data = context->acquire(std::chrono::milliseconds(100));
	//	resultsQueue.push(data);
	//	cout << "push " << i << endl;
	//	sig.notify_one();
	//}
	//context->stop();
	//should_exit = true;
}

static void publish_worker(std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, DataPublisher publisher)
{
	//int process_index = 0;
	//vector<AcquiredData::TriggerData> vec;
	//uint64_t average_processing_time = 0;

	//while (!should_exit)
	//{
	//	{
	//		std::unique_lock<std::mutex> lock(lockable);
	//		sig.wait(lock);
	//	}

	//	while (!workQueue.empty())
	//	{
	//		AcquiredData ae = workQueue.front();
	//		workQueue.pop();
	//		auto result = ae.process(1, 100 * process_index++);
	//		//publisher.process(std::move(result));
	//	}
	//}
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

static void publish_worker_frame(std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, UimfFrame uimf)
{
	//UimfWriter writer(uimf.file_name);
	//int process_index = 0;

	//while (!should_exit)
	//{
	//	{
	//		std::unique_lock<std::mutex> lock(lockable);
	//		sig.wait(lock);
	//	}

	//	while (!workQueue.empty())
	//	{
	//		AcquiredData ae = workQueue.front();
	//		workQueue.pop();
	//		cout << "pop " << process_index << endl;
	//		auto result = ae.process(1, 100 * process_index++);
	//		uimf.append_encoded_results(std::move(result));
	//	}
	//}

	//writer.write_frame(uimf);
}