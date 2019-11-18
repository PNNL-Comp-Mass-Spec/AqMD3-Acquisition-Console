// FalkorAcquisition.cpp : Defines the entry point for the application.

#include "../include/server.h"
#include "../include/digitizer.h"
#include "../include/acquireddata.h"
#include "../include/sa220.h"
#include "../include/acquisitioncontrol.h"
#include "../include/acquirepublisher.h"
#include "../include/acquireframepublisher.h"

#include "../include/uimfframesubscriber.h"
#include "../include/zmqacquireddatasubscriber.h"
#include "../include/processsubject.h"

#include "../include/diagnostic/falsedatapublisher.h"
#include "../include/diagnostic/rawprintersubscriber.h"
#include "../include/diagnostic/simplepublisher.h"
#include "../include/diagnostic/encodeddatawriter.h"

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

//#define print_raw

using namespace std;

static std::tuple<uint64_t, uint64_t> get_tof_width(SA220& digitizer, double sample_rate);
static std::tuple<uint64_t, uint64_t> get_optimal_record_size(SA220& digitizer, uint64_t pusher_pulse_pulse_width_samples, double post_trigger_delay_s, double sample_rate, double trig_rearm_s);
static uint64_t get_trigger_time_stamp_average(SA220& digitizer, int triggers);

static char ack[] = "ack";
static double post_trigger_delay = 0.00001;
static double estimated_trigger_rearm_time = 0.0000001;
uint32_t calculated_post_trigger_samples = 0;

int main(int argc, char *argv[]) {
	try
	{
		auto server = new Server("tcp://*:5555");
		SA220 digitizer("PXI3::0::0::INSTR", "Simulate=false, DriverSetup= Model=SA220P");

		double sampling_rate = 0.0;
		std::unique_ptr<AcquisitionControl> controller;

		///* server stuff */
		server->register_handler([&](Server::ReceivedRequest req)
		{
			for (const auto& command : req.payload)
			{
				std::cout << "\t" << command << std::endl;

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
					continue;
				}

				if (command == "init")
				{
					digitizer.set_trigger_parameters(digitizer.trigger_external, 0.5, true, post_trigger_delay);

					req.send_response(ack);
					continue;
				}

				if (command == "horizontal")
				{
					if (req.payload.size() == 2)
					{
						auto horizontal_resolution = std::stod(req.payload[1]);
						sampling_rate = 1.0 / horizontal_resolution;
						std::cout << "sampling rate: " << sampling_rate << std::endl;
						digitizer.set_sampling_rate(sampling_rate);
					}

					req.send_response(ack);
					return;
				}

				if (command == "vertical")
				{
					if (req.payload.size() == 2)
					{
						auto offset_v = std::stod(req.payload[1]);
						digitizer.set_channel_parameters(digitizer.channel_1, digitizer.full_scale_range_0_5v, offset_v);
					}

					req.send_response(ack);
					return;
				}

				if (command == "trig class")
				{
					// TODO trig class
					continue;
				}

				if (command == "trig source")
				{
					// TODO trig source
					continue;
				}

				if (command == "mode")
				{
					// TODO mode
					continue;
				}

				if (command == "config digitizer")
				{
					// TODO config digitizer
					continue;
				}

				if (command == "post samples")
				{
					continue;
				}

				if (command == "pre samples")
				{
					continue;
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

						std::shared_ptr<UimfFrame> frame = std::make_shared<UimfFrame>(
							uimf.start_trigger(),
							uimf.nbr_samples(),
							uimf.nbr_accumulations(),
							uimf.frame_length(),
							uimf.frame_number(),
							uimf.offset_bins(),
							uimf.file_name()
							);

						uint64_t record_size = frame->nbr_samples - calculated_post_trigger_samples;
						std::cout << "samples per trigger: " << frame->nbr_samples << std::endl;
						std::cout << "record size: " << record_size << std::endl;
						std::cout << "post trigger samples: " << calculated_post_trigger_samples << std::endl;
						digitizer.set_record_size(record_size);

						auto context = digitizer.configure_cst_zs1(digitizer.channel_1, 100, record_size, Digitizer::ZeroSuppressParameters(-32468, 300));
						auto data_pub = server->get_publisher("tcp://*:5554");
						
						std::unique_ptr<AcquireFramePublisher> p = std::make_unique<AcquireFramePublisher>(std::move(context), frame);

						std::shared_ptr<ZmqAcquiredDataSubscriber> vzws = std::make_shared<ZmqAcquiredDataSubscriber>(data_pub, frame->nbr_samples);
						std::shared_ptr<ProcessSubject> ps = std::make_shared<ProcessSubject>(frame, data_pub);
						std::shared_ptr<UimfFrameSubscriber> ufs = std::make_shared<UimfFrameSubscriber>(frame);
						ps->register_subscriber(ufs, SubscriberType::ACQUIRE_FRAME);
						ps->register_subscriber(vzws, SubscriberType::ACQUIRE);
						p->register_subscriber(ps, SubscriberType::ACQUIRE_FRAME);

#ifdef print_raw
						std::shared_ptr<EncodedDataWriter> edw = std::make_shared<EncodedDataWriter>("output_data.txt");
						std::shared_ptr<RawPrinterSubscriber> rps = std::make_shared<RawPrinterSubscriber>("output_ts.txt", 0);

						ps->register_subscriber(edw, SubscriberType::ACQUIRE_FRAME);
						p->register_subscriber(rps,  SubscriberType::ACQUIRE_FRAME);
#endif

						//if (controller) controller.reset();
						controller = std::move(p);
						controller->start();
					}

					req.send_response(ack);
					return;
				}

				if (command == "acquire")
				{
					uint64_t record_size;
					uint64_t post_trigger_samples;
					std::tie(post_trigger_samples, record_size) = get_tof_width(digitizer, sampling_rate);			
					std::cout << "samples per trigger: " << record_size + post_trigger_samples << std::endl;
					std::cout << "record size: " << record_size << std::endl;
					std::cout << "post trigger samples: " << post_trigger_samples << std::endl;
					digitizer.set_record_size(record_size);

					auto context = digitizer.configure_cst_zs1(digitizer.channel_1, 100, record_size, Digitizer::ZeroSuppressParameters(-32468, 300));
					calculated_post_trigger_samples = post_trigger_samples;
					auto data_pub = server->get_publisher("tcp://*:5554");
					std::unique_ptr<AcquirePublisher> p = std::make_unique<AcquirePublisher>(std::move(context));

					std::shared_ptr<ZmqAcquiredDataSubscriber> vzws = std::make_shared<ZmqAcquiredDataSubscriber>(data_pub, record_size + post_trigger_samples);
					std::shared_ptr<ProcessSubject> ps = std::make_shared<ProcessSubject>(post_trigger_samples);
					ps->register_subscriber(vzws, SubscriberType::ACQUIRE);
					p->register_subscriber(ps, SubscriberType::ACQUIRE);

					controller = std::move(p);
					controller->start();

					vector<string> to_send(2);

					TofWidthMessage tofMsg;
					tofMsg.set_num_samples(record_size + post_trigger_samples);
					tofMsg.set_pusher_pulse_width(6125);
					to_send[0] = (tofMsg.SerializeAsString());
					vector<uint8_t> hash(picosha2::k_digest_size);
					picosha2::hash256(to_send[0].begin(), to_send[0].end(), hash.begin(), hash.end());

					to_send[1] = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
					req.send_responses(to_send);

					return;
				}

				if (command == "tof width")
				{
					uint64_t record_size;
					uint64_t post_trigger_samples;
					std::tie(post_trigger_samples, record_size) = get_tof_width(digitizer, sampling_rate);
					std::cout << "samples per trigger: " << record_size + post_trigger_samples << std::endl;
					std::cout << "record size: " << record_size << std::endl;
					std::cout << "post trigger samples: " << post_trigger_samples << std::endl;
					digitizer.set_record_size(record_size);

					vector<string> to_send(2);

					TofWidthMessage tofMsg;
					tofMsg.set_num_samples(record_size + post_trigger_samples);
					tofMsg.set_pusher_pulse_width(6125);
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
						digitizer.disable_io_port();
					}

					req.send_response(ack);
					return;
				}
			}
		});

		//thread t([&] { server->run(); });

		server->run();

		//should_exit = true;

		//server->stop();
		//t.join();
	}
	catch (std::string ex)
	{
		cout << ex;
	}

	return 0;
}

static std::tuple<uint64_t, uint64_t> get_tof_width(SA220& digitizer, double sample_rate)
{
	auto samples_per_trigger = get_trigger_time_stamp_average(digitizer, 20);

	return get_optimal_record_size(digitizer, samples_per_trigger, post_trigger_delay, sample_rate, estimated_trigger_rearm_time);
}

static uint64_t get_trigger_time_stamp_average(SA220& digitizer, int triggers)
{
	digitizer.set_record_size(1024);
	auto dig_context = digitizer.configure_cst(digitizer.channel_1, triggers, 1024);

	dig_context->start();
	AcquiredData result = dig_context->acquire(std::chrono::milliseconds(80));
	dig_context->stop();

	uint64_t total = 0;

	for (int i = 0; i < result.stamps.size() - 1; i++)
	{
		total += result.stamps[i + 1].timestamp - result.stamps[i].timestamp;
	}

	total = ((total / (result.stamps.size() - 1)));

	return total;
}

static std::tuple<uint64_t, uint64_t> get_optimal_record_size(SA220& digitizer, uint64_t pusher_pulse_pulse_width_samples, double post_trigger_delay_s, double sample_rate, double trig_rearm_s)
{
	uint64_t actual_trigger_width_samples = uint64_t(double(pusher_pulse_pulse_width_samples) * (sample_rate / digitizer.max_sample_rate));
	uint64_t est_trig_rearm_samples = uint64_t(trig_rearm_s * sample_rate);
	uint64_t delay_samples = uint64_t(post_trigger_delay_s * sample_rate);

	auto record_size_samples = actual_trigger_width_samples - delay_samples - est_trig_rearm_samples;
	if (record_size_samples % 32 != 0)
		record_size_samples = (record_size_samples / 32) * 32;

	return std::make_tuple(delay_samples, record_size_samples);
}