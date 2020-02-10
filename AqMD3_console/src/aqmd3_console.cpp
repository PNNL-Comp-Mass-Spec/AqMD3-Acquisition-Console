#include "../include/server.h"
#include "../include/digitizer.h"
#include "../include/acquireddata.h"
#include "../include/sa220.h"
#include "../include/acquisitioncontrol.h"
#include "../include/acquirepublisher.h"
#include "../include/uimfframewritersubscriber.h"
#include "../include/zmqacquireddatasubscriber.h"
#include "../include/processsubject.h"
#include "../include/definitions.h"

#include "../include/PicoSHA2/picosha2.h"

#include "../include/diagnostic/uimfframedecompressorsubscriber.h"
#include "../include/diagnostic/encodeddatawriter.h"
#include "../include/diagnostic/rawprintersubscriber.h"

#include <UIMFWriter/uimfwriter.h>
#include "AqMD3.h"
#include <visa.h>
#include <snappy.h>
#include <zmq.hpp>
#include <iostream>
#include <tuple>
#include <string>
#include <iostream>
using std::cerr;
#include <windows.h>

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

using namespace std;

static std::tuple<uint64_t, uint64_t, uint64_t> get_tof_width(const SA220 *digitizer, double sample_rate);
static std::tuple<uint64_t, uint64_t> get_optimal_record_size(const SA220 *digitizer, uint64_t pusher_pulse_pulse_width_samples, double post_trigger_delay_s, double sample_rate, double trig_rearm_s);
static uint64_t get_trigger_time_stamp_average(const SA220 *digitizer, int triggers);

void disable_quick_edit()
{
	HANDLE handle;
	DWORD current_settings;
	handle = GetStdHandle(STD_INPUT_HANDLE);

	if (handle == INVALID_HANDLE_VALUE)
		throw std::string("Error getting standard device handle\n");

	if (!GetConsoleMode(handle, &current_settings))
		throw std::string("Error getting console mode");

	if (!SetConsoleMode(handle, ENABLE_EXTENDED_FLAGS | (current_settings & ~ENABLE_QUICK_EDIT_MODE)))
		throw std::string("Error setting console mode");
}

static char ack[] = "ack";
static double post_trigger_delay = 0.00001;
static double estimated_trigger_rearm_time = 0.0000001;
uint32_t calculated_post_trigger_samples = 0;
uint64_t avg_tof_period_samples = 0;

int main(int argc, char *argv[]) {

	// Disable 'Quick Edit Mode' since it can cause the application to hang during acquisition
	disable_quick_edit();

	std::unique_ptr<SA220> digitizer = std::make_unique<SA220>("PXI1::0::0::INSTR", "Simulate=false, DriverSetup= Model=SA220P");
	auto server = new Server("tcp://*:5555");
	double sampling_rate = 0.0;
	std::unique_ptr<AcquisitionControl> controller;

#if REUSABLE_PUB_SUB
	int record_size_c = 0;
	std::shared_ptr<StreamingContext> context;
	std::shared_ptr<ZmqAcquiredDataSubscriber> zmq_publisher;
	std::shared_ptr<UimfFrameWriterSubscriber> frame_writer = std::make_shared<UimfFrameWriterSubscriber>();
#endif // reusable_pub_sub

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
				std::string info = digitizer->get_model();

				req.send_response(info);
			}

			if (command == "init")
			{
				digitizer->set_trigger_parameters(digitizer->trigger_external, 2.0, true, post_trigger_delay);

				req.send_response(ack);
				continue;
			}

			if (command == "horizontal")
			{
				if (req.payload.size() == 2)
				{
					auto horizontal_resolution = std::stod(req.payload[1]);
					sampling_rate = 1.0 / horizontal_resolution;
					//std::cout << "sampling rate: " << sampling_rate << std::endl;
					digitizer->set_sampling_rate(sampling_rate);
				}

				req.send_response(ack);
				return;
			}

			if (command == "vertical")
			{
				if (req.payload.size() == 2)
				{
					auto offset_v = std::stod(req.payload[1]);
					digitizer->set_channel_parameters(digitizer->channel_1, digitizer->full_scale_range_0_5v, offset_v);
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

				req.send_response(ack);
				return;
			}

			if (command == "acquire frame")
			{
				if (req.payload.size() == 2)
				{
#if TIMING_INFORMATION
					//auto t_0 = std::chrono::high_resolution_clock::now();
#endif
					std::string uimf_req_msg;
					snappy::Uncompress(req.payload[1].data(), req.payload[1].size(), &uimf_req_msg);
					auto uimf = UimfRequestMessage();
					uimf.MergeFromString(uimf_req_msg);

					uint64_t record_size = uimf.nbr_samples() - calculated_post_trigger_samples;
					//std::cout << "samples per trigger: " << uimf.nbr_samples() << std::endl;
					//std::cout << "record size: " << record_size << std::endl;
					//std::cout << "post trigger samples: " << calculated_post_trigger_samples << std::endl;

					digitizer->set_record_size(record_size);

					auto data_pub = server->get_publisher("tcp://*:5554");

#if !REUSABLE_PUB_SUB
					auto context = digitizer->configure_cst_zs1(digitizer->channel_1, 100, record_size, Digitizer::ZeroSuppressParameters(-32667, 100));
					std::shared_ptr<ZmqAcquiredDataSubscriber> zmq_publisher = std::make_shared<ZmqAcquiredDataSubscriber>(data_pub, uimf.nbr_samples());
#endif	

					std::unique_ptr<AcquirePublisher> p = std::make_unique<AcquirePublisher>(context);

					if (calculated_post_trigger_samples <= 0)
					{
						std::cerr << "Calculated post-trigger samples must be greater than 0. Post-trigger samples: " << calculated_post_trigger_samples << std::endl;
						break;
					}

					std::shared_ptr<ProcessSubject> ps = std::make_shared<ProcessSubject>(uimf, data_pub, calculated_post_trigger_samples, avg_tof_period_samples);
					double ts_period = 1.0 / digitizer->max_sample_rate;

#if REUSABLE_PUB_SUB
					ps->FramePublisher<frame_ptr>::register_subscriber(frame_writer, SubscriberType::ACQUIRE_FRAME);
#else
					ps->FramePublisher<frame_ptr>::register_subscriber(std::make_shared<UimfFrameWriterSubscriber>(), SubscriberType::ACQUIRE_FRAME);
#endif

					ps->FramePublisher<segment_ptr>::register_subscriber(zmq_publisher, SubscriberType::ACQUIRE);
					p->register_subscriber(ps, SubscriberType::ACQUIRE_FRAME);

#if PRINT_RAW
					std::shared_ptr<RawPrinterSubscriber> rps = std::make_shared<RawPrinterSubscriber>("timestamp_data_" + std::to_string(uimf.frame_number()));
					std::shared_ptr<EncodedDataWriter> edw = std::make_shared<EncodedDataWriter>("encoded_data_" + std::to_string(uimf.frame_number()));
					std::shared_ptr<UimfFrameDecompressorSubscriber> fws = std::make_shared<UimfFrameDecompressorSubscriber>("frame_data_" + std::to_string(uimf.frame_number()));

					p->FramePublisher<AcquiredData>::register_subscriber(rps, SubscriberType::ACQUIRE_FRAME);
					ps->FramePublisher<segment_ptr>::register_subscriber(edw, SubscriberType::ACQUIRE_FRAME);
					ps->FramePublisher<frame_ptr>::register_subscriber(fws, SubscriberType::ACQUIRE_FRAME);
#endif


					//if (controller) controller.reset();
					controller = std::move(p);
					controller->start(uimf);
#if TIMING_INFORMATION
					//auto t_1 = std::chrono::high_resolution_clock::now();
					//auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t_1 - t_0);
					//std::cout << "time to setup acquire frame:" << diff.count() << "\n";
#endif
				}

				req.send_response(ack);
				return;
			}

			if (command == "acquire")
			{
				uint64_t record_size;
				uint64_t post_trigger_samples;
				uint64_t tof_width;
				std::tie(post_trigger_samples, record_size, tof_width) = get_tof_width(digitizer.get(), sampling_rate);			
				std::cout << "samples per trigger: " << record_size + post_trigger_samples << std::endl;
				std::cout << "record size: " << record_size << std::endl;
				std::cout << "post trigger samples: " << post_trigger_samples << std::endl;
				digitizer->set_record_size(record_size);
				avg_tof_period_samples = tof_width;
				calculated_post_trigger_samples = post_trigger_samples;

				auto data_pub = server->get_publisher("tcp://*:5554");


#if REUSABLE_PUB_SUB
				context = digitizer->configure_cst_zs1(digitizer->channel_1, 100, record_size, Digitizer::ZeroSuppressParameters(-32667, 100), 80);
				zmq_publisher = std::make_shared<ZmqAcquiredDataSubscriber>(data_pub, record_size + post_trigger_samples);
#else
				auto context = digitizer->configure_cst_zs1(digitizer->channel_1, 100, record_size, Digitizer::ZeroSuppressParameters(-32667, 100));
				auto zmq_publisher = std::make_shared<ZmqAcquiredDataSubscriber>(data_pub, record_size + post_trigger_samples);
#endif	
				std::unique_ptr<AcquirePublisher> p = std::make_unique<AcquirePublisher>(context);
				std::shared_ptr<ProcessSubject> ps = std::make_shared<ProcessSubject>(post_trigger_samples, tof_width);
				ps->FramePublisher<segment_ptr>::register_subscriber(zmq_publisher, SubscriberType::ACQUIRE);
				p->register_subscriber(ps, SubscriberType::ACQUIRE);

				controller = std::move(p);
				controller->start();

				vector<string> to_send(2);

				TofWidthMessage tofMsg;
				tofMsg.set_num_samples(record_size + post_trigger_samples);

				//'tof_width / (2 * 16)' necessary to work with Falkor for the time being
				tofMsg.set_pusher_pulse_width(tof_width / (2 * 16));				
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
				uint64_t tof_width;
				std::tie(post_trigger_samples, record_size, tof_width) = get_tof_width(digitizer.get(), sampling_rate);
				std::cout << "samples per trigger: " << record_size + post_trigger_samples << std::endl;
				std::cout << "record size: " << record_size << std::endl;
				std::cout << "post trigger samples: " << post_trigger_samples << std::endl;
				digitizer->set_record_size(record_size);

				vector<string> to_send(2);

				TofWidthMessage tofMsg;
				tofMsg.set_num_samples(record_size + post_trigger_samples);
				tofMsg.set_pusher_pulse_width(tof_width / (2 * 16));
				to_send[0] = (tofMsg.SerializeAsString());
				vector<uint8_t> hash(picosha2::k_digest_size);
				picosha2::hash256(to_send[0].begin(), to_send[0].end(), hash.begin(), hash.end());

				to_send[1] = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
				req.send_responses(to_send);

				return;
			}

			if (command == "stop")
			{
				try
				{
					if (controller)
					{
#if TIMING_INFORMATION
						//auto t_0 = std::chrono::high_resolution_clock::now();
#endif
						controller->stop();
						//controller.reset();
#if TIMING_INFORMATION
						//auto t_1 = std::chrono::high_resolution_clock::now();
						//auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t_1 - t_0);
						//std::cout << "time to stop:" << diff.count() << "\n";
#endif
					}
				}
				catch (...)
				{
					std::cout << "problem stopping the application" << std::endl;
				}

				req.send_response(ack);
				return;
			}

			if (command == "invert")
			{
				if (req.payload.size() == 2)
				{
					bool invert = req.payload[1] == "true";
					digitizer->set_channel_data_inversion(digitizer->channel_1, invert);
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
					digitizer->enable_io_port();
				}

				req.send_response(ack);
				return;
			}

			if (command == "disable io port")
			{
				if (req.payload.size() == 2)
				{
					auto val = std::stoi(req.payload[1]);
					digitizer->disable_io_port();
				}

				req.send_response(ack);
				return;
			}
		}
	});

	server->run();

	return 0;
}

static std::tuple<uint64_t, uint64_t, uint64_t> get_tof_width(const SA220 * digitizer, double sample_rate)
{
	auto samples_per_trigger = get_trigger_time_stamp_average(digitizer, 20);

	return std::tuple_cat(
		get_optimal_record_size(digitizer, samples_per_trigger, post_trigger_delay, sample_rate, estimated_trigger_rearm_time),
		std::make_tuple(samples_per_trigger)
		);
}

static uint64_t get_trigger_time_stamp_average(const SA220 *digitizer, int triggers)
{
	digitizer->set_record_size(1024);
	auto dig_context = digitizer->configure_cst(digitizer->channel_1, triggers, 1024, 10);

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

static std::tuple<uint64_t, uint64_t> get_optimal_record_size(const SA220 *digitizer, uint64_t pusher_pulse_pulse_width_samples, double post_trigger_delay_s, double sample_rate, double trig_rearm_s)
{
	uint64_t actual_trigger_width_samples = uint64_t(double(pusher_pulse_pulse_width_samples) * (sample_rate / digitizer->max_sample_rate));
	uint64_t est_trig_rearm_samples = uint64_t(trig_rearm_s * sample_rate);
	uint64_t delay_samples = uint64_t(post_trigger_delay_s * sample_rate);

	auto record_size_samples = actual_trigger_width_samples - delay_samples - est_trig_rearm_samples;
	if (record_size_samples % 32 != 0)
		record_size_samples = (record_size_samples / 32) * 32;

	return std::make_tuple(delay_samples, record_size_samples);
}