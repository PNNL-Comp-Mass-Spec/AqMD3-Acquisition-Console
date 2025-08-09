#include "../include/server.h"
#include "../include/acquisitioncontrol.h"
#include "../include/acquirepublisher.h"
#include "../include/uimfframewritersubscriber.h"
#include "../include/zmqacquireddatasubscriber.h"
#include "../include/processsubject.h"
#include "../include/definitions.h"
#include "../include/util/config.h"
#include "../include/util/uimfhelpers.h"
#include "../include/message.pb.h"
#include "../include/massspec/toftiminginformation.h"
#include "include/app.h"

#include <libaqmd3/digitizer.h>
#include <libaqmd3/acquireddata.h>
#include <libaqmd3/sa220.h>

#include <UIMFWriter/uimfwriter.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>

#include <stdexcept>
#include <picosha2.h>
#include <visa.h>
#include <snappy.h>
#include <zmq.hpp>
#include <iostream>
#include <tuple>
#include <string>
using std::cerr;
#include <windows.h>
#include "../include/diagnostic/datageneratorcontext.h"

#define NOMINMAX 
#undef min
#undef max


void disable_quick_edit()
{
	HANDLE handle;
	DWORD current_settings;
	handle = GetStdHandle(STD_INPUT_HANDLE);

	if (handle == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("Error getting standard device handle\n");
	}

	if (!GetConsoleMode(handle, &current_settings))
	{
		throw std::runtime_error("Error getting console mode");
	}

	if (!SetConsoleMode(handle, ENABLE_EXTENDED_FLAGS | (current_settings & ~ENABLE_QUICK_EDIT_MODE)))
	{
		throw std::runtime_error("Error setting console mode");
	}
}

static char ack[] = "ack";
uint32_t calculated_post_trigger_samples = 0;
uint64_t avg_tof_period_samples = 0;

// Configuration variables
static double post_trigger_delay = 0.00001; // Default post trigger delay in 10us
static double estimated_trigger_rearm_time = 0.000002048; // Default allowed trigger rearm time is 2us
uint64_t notify_on_scans_count = 500;
std::string resource_name = "PXI0::0::0::INSTR";
int64_t acquisition_timeout_ms = 100;
uint64_t acquisition_initial_buffer_count = 40;
uint64_t acquisition_max_buffer_count = 100;
//int32_t trigger_events_per_read_count = 100;
uint64_t acquisition_buffer_reserve_elements_count = 2048;


static void print_config_value(const std::string& key, const std::string &value, bool is_found) {
	std::string msg = "Config value \"" + key;
	if (is_found)
	{
		msg += "\" found, value set to " + value;
	}
	else
	{
		msg += " not found, value defaulted to " + value;
	}

	spdlog::info(msg);
}

void configure_logger()
{
	try 
	{
		auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/log", 2, 0);
		daily_sink->set_level(spdlog::level::trace);

		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::trace);

		auto logger = std::make_shared<spdlog::logger>("aqmd3", spdlog::sinks_init_list( { daily_sink, console_sink }));
		logger->set_level(spdlog::level::trace);
		spdlog::set_default_logger(logger);

		spdlog::info("Logger initialized");
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log init failed: " << ex.what() << std::endl;
	}
}

void configure_settings()
{
	auto config = Config("config.txt");
	bool has_config = config.exists();
	config.read();

	auto config_found = (has_config ? "config found\n" : "no config found, using default values\n");
	spdlog::info(config_found);

	post_trigger_delay = config.has_key("PostTriggerDelay") ? std::stod(config.get_value("PostTriggerDelay")) : post_trigger_delay;
	print_config_value("PostTriggerDelay", std::to_string(post_trigger_delay), config.has_key("PostTriggerDelay"));

	estimated_trigger_rearm_time = config.has_key("TriggerRearmDeadTime") ? std::stod(config.get_value("TriggerRearmDeadTime")) : estimated_trigger_rearm_time;
	print_config_value("TriggerRearmDeadTime", std::to_string(estimated_trigger_rearm_time), config.has_key("TriggerRearmDeadTime"));

	resource_name = config.has_key("ResourceName") ? config.get_value("ResourceName") : resource_name;
	print_config_value("ResourceName", resource_name, config.has_key("ResourceName"));

	notify_on_scans_count = config.has_key("NotifyOnScansCount") ? std::stoull(config.get_value("NotifyOnScansCount")) : notify_on_scans_count;
	print_config_value("NotifyOnScansCount", std::to_string(notify_on_scans_count), config.has_key("NotifyOnScansCount"));

	acquisition_timeout_ms = config.has_key("AcquisitionTimeoutMs") ? std::stoll(config.get_value("AcquisitionTimeoutMs")) : acquisition_timeout_ms;
	print_config_value("AcquisitionTimeoutMs", std::to_string(acquisition_timeout_ms), config.has_key("AcquisitionTimeoutMs"));

	acquisition_initial_buffer_count = config.has_key("AcquisitionInitialBufferCount") ? std::stoull(config.get_value("AcquisitionInitialBufferCount")) : acquisition_initial_buffer_count;
	print_config_value("AcquisitionInitialBufferCount", std::to_string(acquisition_initial_buffer_count), config.has_key("AcquisitionInitialBufferCount"));

	acquisition_max_buffer_count = config.has_key("AcquisitionMaxBufferCount") ? std::stoull(config.get_value("AcquisitionMaxBufferCount")) : acquisition_max_buffer_count;
	print_config_value("AcquisitionMaxBufferCount", std::to_string(acquisition_max_buffer_count), config.has_key("AcquisitionMaxBufferCount"));

	//trigger_events_per_read_count = config.has_key("TriggerEventsPerReadCount") ? std::stoi(config.get_value("TriggerEventsPerReadCount")) : trigger_events_per_read_count;
	//print_config_value("TriggerEventsPerReadCount", std::to_string(trigger_events_per_read_count), config.has_key("TriggerEventsPerReadCount"));

	acquisition_buffer_reserve_elements_count = config.has_key("AcquisitionBufferReserveElementsCount") ? std::stoull(config.get_value("AcquisitionBufferReserveElementsCount")) : acquisition_buffer_reserve_elements_count;
	print_config_value("AcquisitionBufferReserveElementsCount", std::to_string(acquisition_buffer_reserve_elements_count), config.has_key("AcquisitionBufferReserveElementsCount"));
}

int main(int argc, char *argv[]) {
	try
	{
		// Disable 'Quick Edit Mode' since it can cause the application to hang during acquisition
		disable_quick_edit();
		configure_logger();
		configure_settings();

#if TEST_ACQUIRE
		std::unique_ptr<SA220> digitizer = std::make_unique<SA220>(resource_name, true);
#else
		std::unique_ptr<SA220> digitizer = std::make_unique<SA220>(resource_name, false);
#endif

		auto server = new Server("tcp://*:5555");
		double sampling_rate = 0.0;
		std::unique_ptr<AcquisitionControl> controller;
		std::shared_ptr<AcquisitionBufferPool> buffer_pool = nullptr;

#if REUSABLE_PUB_SUB
		int record_size_c = 0;
		std::shared_ptr<StreamingContext> context;
		std::shared_ptr<ZmqAcquiredDataSubscriber> zmq_publisher;
		std::shared_ptr<UimfFrameWriterSubscriber> frame_writer = std::make_shared<UimfFrameWriterSubscriber>(false);
#endif // reusable_pub_sub

		server->register_handler([&](Server::ReceivedRequest req)
			{
				for (const auto& command : req.payload)
				{
					spdlog::info("command: " + command);

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
						auto info = digitizer->get_info();
						auto info_str = std::format("Digitizer Model: {} / Digitizer Serial No.: {} / Digitizer Firmware Version: {} / App: {} / App Version: {}-{}",
							info.instrument_model,
							info.serial_number,
							info.firmware_revision,
							PROJECT_NAME_S,
							AqMD3_console_VERSION_S,
							GIT_COMMIT_HASH);
						req.send_response(info_str);
					}

					if (command == "firmware")
					{
						auto info = digitizer->get_info();
						req.send_response(info.firmware_revision);
					}

					if (command == "serial")
					{
						auto info = digitizer->get_info();
						req.send_response(info.serial_number);
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
							auto t_0 = std::chrono::high_resolution_clock::now();
#endif
							std::string uimf_req_msg;
							snappy::Uncompress(req.payload[1].data(), req.payload[1].size(), &uimf_req_msg);
							auto uimf = UimfRequestMessage();
							uimf.MergeFromString(uimf_req_msg);

							// Start acquire. Waits for external enabe signal.
							auto uimf_frame_params = UIMFHelpers::uimf_message_to_parameters(uimf);
							UIMFHelpers::log_info_uimf_frame_params(uimf_frame_params);
							controller->start(uimf_frame_params);

#if TIMING_INFORMATION
							auto t_1 = std::chrono::high_resolution_clock::now();
							auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t_1 - t_0);
							std::cout << "time to setup acquire frame:" << diff.count() << "\n";
#endif
						}

						req.send_response(ack);
						return;
					}

					if (command == "acquire")
					{
#if TEST_ACQUIRE
						auto timing = AqirisDigitizer::TofTimingInformation(
							200000,
							200000 - 20000 - 20000,
							20000,
							20000,
							post_trigger_delay,
							estimated_trigger_rearm_time);
#else
						auto timing = AqirisDigitizer::TofTimingInformation::create_timing_information(digitizer.get(), sampling_rate, post_trigger_delay, estimated_trigger_rearm_time);
#endif
						uint64_t record_size = timing.get_record_size();
						uint64_t post_trigger_samples = timing.get_post_trigger_delay_samples();
						uint64_t tof_width = timing.get_samples_per_trigger();
						spdlog::info("tof width: " + std::to_string(tof_width));
						spdlog::info("samples per trigger: " + std::to_string(record_size + post_trigger_samples));
						spdlog::info("record size: " + std::to_string(record_size));
						spdlog::info("post trigger samples: " + std::to_string(post_trigger_samples));
						digitizer->set_record_size(record_size);
						avg_tof_period_samples = tof_width;
						calculated_post_trigger_samples = post_trigger_samples;

						auto data_pub = server->get_publisher("tcp://*:5554");

						if (buffer_pool == nullptr)
						{
							buffer_pool = std::make_shared<AcquisitionBufferPool>(notify_on_scans_count, avg_tof_period_samples, acquisition_initial_buffer_count, acquisition_max_buffer_count);
						}
#if TEST_ACQUIRE
						context = std::make_shared<DataGeneratorContext>(dynamic_cast<const Digitizer&>(*digitizer), digitizer->channel_1, notify_on_scans_count, buffer_pool);
#else
						context = digitizer->configure_cst(digitizer->channel_1, buffer_pool, Digitizer::ZeroSuppressParameters(-32667, 100));
#endif

						std::unique_ptr<AcquirePublisher> p = std::make_unique<AcquirePublisher>(context, acquisition_timeout_ms, buffer_pool, notify_on_scans_count, data_pub);
						std::shared_ptr<ProcessSubject> ps = std::make_shared<ProcessSubject>(tof_width);
						std::shared_ptr<UimfFrameWriterSubscriber> fw = std::make_shared<UimfFrameWriterSubscriber>(false);
						std::shared_ptr<ZmqAcquiredDataSubscriber> zmq = std::make_shared<ZmqAcquiredDataSubscriber>(data_pub, record_size + post_trigger_samples);
						ps->Publisher<frame_ptr>::register_subscriber(zmq, SubscriberType::ACQUIRE);
						ps->Publisher<frame_ptr>::register_subscriber(fw, SubscriberType::ACQUIRE_FRAME);
						p->register_subscriber(ps, SubscriberType::BOTH);

						controller = std::move(p);

						auto uimf_frame_params = UIMFHelpers::create_inf_params();
						UIMFHelpers::log_info_uimf_frame_params(uimf_frame_params);
						controller->start(uimf_frame_params);

						std::vector<std::string> to_send(2);

						TofWidthMessage tofMsg;
						tofMsg.set_num_samples(record_size + post_trigger_samples);
						tofMsg.set_pusher_pulse_width(tof_width);
						to_send[0] = (tofMsg.SerializeAsString());
						std::vector<uint8_t> hash(picosha2::k_digest_size);

						picosha2::hash256(to_send[0].begin(), to_send[0].end(), hash.begin(), hash.end());

						to_send[1] = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
						req.send_responses(to_send);

						return;
					}

					if (command == "tof width")
					{
						auto timing = AqirisDigitizer::TofTimingInformation::create_timing_information(digitizer.get(), sampling_rate, post_trigger_delay, estimated_trigger_rearm_time);
						uint64_t record_size = timing.get_record_size();
						uint64_t post_trigger_samples = timing.get_post_trigger_delay_samples();
						uint64_t tof_width = timing.get_samples_per_trigger();
						spdlog::info("samples per trigger: " + std::to_string(record_size + post_trigger_samples));
						spdlog::info("record size: " + std::to_string(record_size));
						spdlog::info("post trigger samples: " + std::to_string(post_trigger_samples));
						digitizer->set_record_size(record_size);

						std::vector<std::string> to_send(2);

						TofWidthMessage tofMsg;
						tofMsg.set_num_samples(record_size + post_trigger_samples);
						tofMsg.set_pusher_pulse_width(tof_width);
						to_send[0] = (tofMsg.SerializeAsString());
						std::vector<uint8_t> hash(picosha2::k_digest_size);
						picosha2::hash256(to_send[0].begin(), to_send[0].end(), hash.begin(), hash.end());

						to_send[1] = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
						req.send_responses(to_send);

						return;
					}

					if (command == "stop")
					{
						spdlog::debug(std::format("stop payload size: {}", req.payload.size()));
						if (req.payload.size() != 2)
						{
							return;
						}

						try
						{
							if (controller)
							{
#if TIMING_INFORMATION
								auto t_0 = std::chrono::high_resolution_clock::now();
#endif
								auto stop_acquire = req.payload[1] == "acquire";
								controller->stop(stop_acquire);
#if TIMING_INFORMATION
								auto t_1 = std::chrono::high_resolution_clock::now();
								auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(t_1 - t_0);
								std::cout << "time to stop:" << diff.count() << "\n";
#endif
							}
						}
						catch (const std::exception& ex)
						{
							spdlog::error("Error when trying to stop acquisition loop: " + std::string(ex.what()));
						}
						catch (...)
						{
							spdlog::error("Unknown error when trying to stop acquisition loop");
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
							digitizer->enable_io_port(SA220::control_io_2);
						}

						req.send_response(ack);
						return;
					}

					if (command == "disable io port")
					{
						if (req.payload.size() == 2)
						{
							auto val = std::stoi(req.payload[1]);
							digitizer->disable_io_port(SA220::control_io_2);
						}

						req.send_response(ack);
						return;
					}
				}
			});

		server->run();

		return 0;
	}
	catch (const std::exception& ex)
	{
		spdlog::critical("AqMD3 console application has encountered an error that it cannot recover from, application exiting");
		spdlog::critical(ex.what());
	}
	catch (...)
	{
		spdlog::critical("AqMD3 console application has encountered an unknown error that it cannot recover from, application exiting");
	}

	return 1;
}
