﻿// FalkorAcquisition.cpp : Defines the entry point for the application.

#include "../include/server.h"
#include "../include/digitizer.h"
#include "../include/acquireddata.h"
#include "../include/sa220.h"
#include "../include/acquisitioncontrol.h"
#include "../include/acquirepublisher.h"
#include "../include/zmqdatasubscriber.h"
#include "../include/framewritersubscriber.h"
#include "../include/framesubject.h"

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

static std::pair<uint64_t, double> get_tof_width(SA220& digitizer);

char ack[] = "ack";

int main(int argc, char *argv[]) {
	try
	{
		auto server = new Server("tcp://*:5555");
		SA220 digitizer("PXI3::0::0::INSTR", "Simulate=false, DriverSetup= Model=SA220P");

		cout << "set sample" << endl;
		digitizer.set_sampling_rate(1000000000.0);
		cout << "set trig" << endl;
		digitizer.set_trigger_parameters(digitizer.trigger_external, 0.5, true);
		cout << "set chan" << endl;
		digitizer.set_channel_parameters(digitizer.channel_1, digitizer.full_scale_range_500mv, 0.0);
		vector<thread> threads;
		bool should_exit = false;
		condition_variable signal;
		queue<AcquiredData> dataQueue;
		mutex lock;

		std::unique_ptr<AcquisitionControl> controller;

		int record_size = 94016;
		digitizer.set_record_size(record_size);
		auto context = digitizer.configure_cst_zs1(digitizer.channel_1, 100, record_size, Digitizer::ZeroSuppressParameters(0, 200));
		auto data_pub = server->get_publisher("tcp://*:6546");
		std::unique_ptr<AcquirePublisher> p = std::make_unique<AcquirePublisher>(std::move(context));
		
		//std::shared_ptr<ZmqDataSubscriber> zs = std::make_unique<ZmqDataSubscriber>(std::move(data_pub), record_size);
		//p->register_subscriber(zs, SubscriberType::ACQUIRE);

		//std::shared_ptr<FrameWriterSubscriber> fws = std::make_shared<FrameWriterSubscriber>();

		//std::shared_ptr<UimfFrame> frame = std::make_shared<UimfFrame>(
		//	(uint64_t)0,
		//	(uint64_t)record_size,
		//	(uint64_t)1,
		//	(uint64_t)10000,
		//	(uint32_t)1,
		//	(uint32_t)0,
		//	string("test.uimf")
		//	);
		//std::shared_ptr<FrameSubject> fs = std::make_shared<FrameSubject>(frame, std::move(data_pub));
		//	
		//fs->register_subscriber(fws, SubscriberType::ACQUIRE_FRAME);
		//
		//p->register_subscriber(fs, SubscriberType::ACQUIRE_FRAME);

		//controller = std::move(p);
		//controller->start();

		//std::this_thread::sleep_for(30s);
		//controller->stop();

		///* server stuff */
		server->register_handler([&](Server::ReceivedRequest req)
		{
			std::string command = req.payload[0];
			std::cout << command << std::endl;

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
				return;
			}

			if (command == "pre samples")
			{
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

					std::shared_ptr<UimfFrame> frame = std::make_shared<UimfFrame>(
						uimf.start_trigger(),
						get<0>(tof_params),
						uimf.nbr_accumulations(),
						uimf.frame_length(),
						uimf.frame_number(),
						uimf.offset_bins(),
						uimf.file_name()
					);

					auto context = digitizer.configure_cst_zs1(digitizer.channel_1, 100, get<0>(tof_params), Digitizer::ZeroSuppressParameters(0, 200));

					std::shared_ptr<ZmqDataSubscriber> zs = std::make_unique<ZmqDataSubscriber>(std::move(data_pub), record_size);
					p->register_subscriber(zs, SubscriberType::ACQUIRE);
					std::shared_ptr<FrameWriterSubscriber> fws = std::make_shared<FrameWriterSubscriber>();
					std::shared_ptr<FrameSubject> fs = std::make_shared<FrameSubject>(frame, std::move(data_pub));
					fs->register_subscriber(fws, SubscriberType::ACQUIRE_FRAME);
					p->register_subscriber(fs, SubscriberType::ACQUIRE_FRAME);

					controller = std::move(p);
					controller->start();
				}

				req.send_response(ack);
				return;
			}

			if (command == "acquire")
			{
				std::pair<uint64_t, double> tof_params = get_tof_width(digitizer);
				uint64_t record_size = get<0>(tof_params);
				std::cout << "samples per trigger: " << record_size << std::endl;
				digitizer.set_record_size(record_size);

				auto context = digitizer.configure_cst_zs1(digitizer.channel_1, 100, record_size, Digitizer::ZeroSuppressParameters(0, 200));

				auto data_pub = server->get_publisher("tcp://*:5554");
				std::unique_ptr<AcquirePublisher> p =  std::make_unique<AcquirePublisher>(std::move(context));
				std::shared_ptr<ZmqDataSubscriber> zs = std::make_unique<ZmqDataSubscriber>(std::move(data_pub), record_size);
				p->register_subscriber(zs, SubscriberType::ACQUIRE);

				controller = std::move(p);
				controller->start();

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
				if (controller)
				{
					controller->stop();
				}

				req.send_response(ack);
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

		//server->run();

		getchar();
		should_exit = true;
		std::cout << "stopping -- press again to stop" << endl;

		server->stop();
		t.join();
	}
	catch (std::string ex)
	{
		cout << ex;
	}

	return 0;
}

std::pair<uint64_t, double> get_tof_width(SA220& digitizer)
{
	digitizer.set_record_size(1024);
	auto dig_context = digitizer.configure_cst(digitizer.channel_1, 100, 1024);

	dig_context->start();
	AcquiredData result = dig_context->acquire(std::chrono::milliseconds(80));
	dig_context->stop();

	uint64_t total = 0;

	for (int i = 0; i < result.stamps.size() - 1; i++)
	{
		total += result.stamps[i + 1].timestamp - result.stamps[i].timestamp;
	}

	total = ((total / (result.stamps.size() - 1)) / 2);
	int r = total % 32;
	total = total - r - /* 2 * */ 128;
	double time = total / /*(2 * */ pow(10, 9) /* ) */;

	return std::make_pair(total, time);
}