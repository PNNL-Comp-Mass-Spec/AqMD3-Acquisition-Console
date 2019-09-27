// FalkorAcquisition.cpp : Defines the entry point for the application.

#include "../include/acquisitionbufferpool.h"
#include "../include/server.h"
#include "../include/digitizer.h"
#include "../include/acquisitioncontext.h"
#include "../include/dgtcontext.h"
#include "../include/acquireddata.h"
#include "../include/sa220.h"

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

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
#include <iostream>
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

static void digitizer_worker(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, shared_ptr<StreamingContext> context);
static void publish_worker(zmq::socket_t &pusher, std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, shared_ptr<StreamingContext> context,
	ofstream& readable_file);

bool should_exit = false;

int main() {
	auto server = new Server("tcp://*:5555");
	SA220 digitizer("PXI3::0::0::INSTR", "Simulate=false, DriverSetup= Model=SA220P");

	digitizer.set_record_size(94016);
	digitizer.set_sampling_rate(1000000000.0);
	digitizer.set_trigger_parameters(digitizer.trigger_external, 0.5, true);
	digitizer.set_channel_parameters(digitizer.channel_1, digitizer.full_scale_range_2500mv, 0.0);

	condition_variable signal;
	queue<AcquiredData> dataQueue;
	mutex lock;

	auto dig = digitizer.configure_cst_zs1(digitizer.channel_1, 100, 1400, 100, 0, 0);

	zmq::context_t context(1);
	zmq::socket_t publisher(context, ZMQ_PUB);
	publisher.bind("tcp://*:6546");

	vector<thread> threads;

	ofstream readable_file;
	auto t = std::chrono::system_clock::now();
	auto tp = std::chrono::system_clock::to_time_t(t);
	string tps = std::ctime(&tp);

	readable_file.open("output.txt");

	threads.push_back(thread(publish_worker, ref(publisher), ref(signal), ref(lock), ref(dataQueue), dig, ref(readable_file)));
	threads.push_back(thread(digitizer_worker, ref(signal), ref(dataQueue), dig));

	for (auto& thread : threads)
	{
		thread.join();
	}
	readable_file.close();


	//auto server = new Server("tcp://*:6545");
	//server->register_handler([&](Server::ReceivedRequest req) 
	//{
	//	for (auto& payload = req.payload.begin(); payload != req.payload.end(); payload++)
	//	{
	//		if (*payload == "num instruments")
	//		{
	//			
	//			req.send_response("1");
	//			return;
	//		}

	//		if (*payload == "info")
	//		{
	//			continue;
	//		}

	//		if (*payload == "init")
	//		{
	//			continue;
	//		}

	//		if (*payload == "horizontal")
	//		{
	//			continue;
	//		}

	//		if (*payload == "vertical")
	//		{
	//			continue;
	//		}

	//		if (*payload == "trig class")
	//		{
	//			continue;
	//		}

	//		if (*payload == "trig source")
	//		{
	//			continue;
	//		}

	//		if (*payload == "mode")
	//		{
	//			continue;
	//		}

	//		if (*payload == "config digitizer")
	//		{
	//			continue;
	//		}

	//		if (*payload == "post samples")
	//		{
	//			continue;
	//		}

	//		if (*payload == "pre samples")
	//		{
	//			continue;
	//		}

	//		if (*payload == "setup array")
	//		{
	//			continue;
	//		}

	//		if (*payload == "acquire frame")
	//		{
	//			continue;
	//		}

	//		if (*payload == "acquire")
	//		{
	//		}

	//		if (*payload == "tof width")
	//		{
	//			std::shared_ptr<StreamingContext> dig_context = digitizer.configure_cst(1024);

	//			dig_context->start();
	//			AcquiredData result = dig_context->acquire(256, std::chrono::milliseconds(80));
	//			dig_context->stop();

	//			uint64_t total = 0;
	//			vector<uint64_t> stamps = result.stamps;

	//			for (int i = 0; i < result.stamps.size() - 1; i++)
	//			{
	//				double j = stamps[i + 1] - stamps[i];
	//				total += j;
	//			}

	//			total = total / (result.stamps.size() - 1);

	//			double time = total / (2 * pow(10, 9));
	//			uint64_t samples = time / pow(10, -9);

	//			vector<string> to_send(2);

	//			TofWidthMessage tofMsg;
	//			tofMsg.set_num_samples(samples);
	//			tofMsg.set_pusher_pulse_width(time);

	//			to_send[0] = (tofMsg.SerializeAsString());

	//			vector<uint8_t> hash(picosha2::k_digest_size);
	//			picosha2::hash256(to_send[0].begin(), to_send[0].end(), hash.begin(), hash.end());

	//			to_send[1] = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
	//			req.send_responses(to_send);
	//			
	//			return;
	//		}

	//		if (*payload == "stop")
	//		{
	//			continue;
	//		}

	//		if (*payload == "invert")
	//		{
	//			continue;
	//		}

	//		if (*payload == "reset timestamps")
	//		{
	//			continue;
	//		}

	//		if (*payload == "enable io port")
	//		{
	//			continue;
	//		}

	//		if (*payload == "disable io port")
	//		{
	//			continue;
	//		}
	//	}
	//});

	//thread t([&] { server->run(); });

	//getchar();
	//server->stop();
	//t.join();

	return 0;
}

static void digitizer_worker(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, shared_ptr<StreamingContext> context)
{
	context->start();
	for (int i = 0; i < 4; i++)
	{
		auto t1 = chrono::high_resolution_clock::now();
		auto data = context->acquire(std::chrono::milliseconds(100));
		resultsQueue.push(data);
		auto t2 = chrono::high_resolution_clock::now();
		sig.notify_one();
	}
	should_exit = true;
}

static void publish_worker(zmq::socket_t &pusher, std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, shared_ptr<StreamingContext> context,
	ofstream& readable_file)
{

	int i = 1;
	vector<TriggerData> vec;
	ofstream datasink;
	//datasink.open("nul");

	while (!should_exit)
	{
		{
			std::unique_lock<std::mutex> lock(lockable);
			sig.wait(lock);
		}

		while (!workQueue.empty())
		{
			Message msg;
			msg.mutable_tic()->Reserve(256);
			msg.mutable_time_stamps()->Reserve(256);

			auto t1 = chrono::high_resolution_clock::now();
			AcquiredData ae = workQueue.front();
			workQueue.pop();

			try
			{
				std::vector<uint64_t> timestamps;
				std::vector<uint32_t> tics;
				std::vector<int32_t> mz_sum;

				std::tie(timestamps, mz_sum, tics) = ae.process();

				readable_file << "Timestamps\n";
				readable_file << "[";
				for (auto ts : timestamps)
				{
					readable_file << " " << ts << " ";
				}
				readable_file << "]\n";

				readable_file << "tic\n";
				readable_file << "[";
				for (auto tic : tics)
				{
					readable_file << " " << tic << " ";
				}
				readable_file << "]\n";

				readable_file << "Samples\n";
				readable_file << "[";
				for (auto samp : mz_sum)
				{
					readable_file << " " << samp << " ";
				}
				readable_file << "]\n";
			}
			catch (const std::exception& e) {
				std::cout << e.what();
			}
			auto t2 = chrono::high_resolution_clock::now();
			cout << "\tprocess: " << (t2 - t1).count() << endl;
		}
	}
	datasink.close();
	context->stop();
}

//static void publish_worker(zmq::socket_t &pusher, std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, shared_ptr<StreamingContext> context,
//	ofstream& readable_file)
//{
//
//	int i = 1;
//	vector<TriggerData> vec;
//	ofstream datasink;
//	//datasink.open("nul");
//
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
//			try
//			{
//				auto result = ae.process();
//				readable_file << "total scans: " << result.size() << endl;
//				for (int i = 0; i < result.size(); i++)
//				{
//					auto trigger = result[i];
//					//auto lam = [](int& tot, DataSegment& seg) {return tot + seg.peak_data.size(); };
//					//auto actual = accumulate(trigger.segments.begin(),
//					//	trigger.segments.end(), 0, lam);
//
//					readable_file << "\nScan: " << i
//						<< " | Timestamp (samples): " << trigger.timestamp
//						<< " | Timestamp (s): " << std::setprecision(12) << (trigger.timestamp / 2000000000.0)
//						<< " | Gate Count: " << trigger.gate_data.size();
//						//<< " | Acquired Samples: " << acqd
//					
//					auto td = (i > 0) ? (result[i].timestamp - result[i - 1].timestamp) / (2000000000.0) : -1.0;
//					readable_file << " | time diff (trigger(i) - trigger(i-1) (s)): " << std::setprecision(12) << td;
//
//					readable_file << "\n";
//					for (auto& data : trigger.segments)
//					{
//						size_t size = data.peak_data.size();
//						readable_file << "Sample Start Index: " << data.first_index
//							<< " | Sample data length: " << size << "\n";
//
//						readable_file << "\[";
//						//for (int i = 0; i < data.peak_data.size() / 8; i++)
//						//{
//						//	//std::cout << "writing to nul\n";
//						//	auto v = data.peak_data;
//						//	//datasink << " "
//						//	if (i == 0)
//						//		readable_file << "";
//						//	else
//						//		readable_file << " ";
//
//						//	readable_file << v[i * 8 + 0] << " "
//						//		<< v[i * 8 + 1] << " "
//						//		<< v[i * 8 + 2] << " "
//						//		<< v[i * 8 + 3] << " "
//						//		<< v[i * 8 + 4] << " "
//						//		<< v[i * 8 + 5] << " "
//						//		<< v[i * 8 + 6] << " "
//						//		<< v[i * 8 + 7];
//
//						//	if (i != data.peak_data.size() - 1)
//						//		readable_file << "\n";
//						//	else
//						//		readable_file << " ]";
//						//}
//						for (int i = 0; i < data.peak_data.size() / 8; i++)
//						{
//							//std::cout << "writing to nul\n";
//							auto v = data.peak_data;
//							//datasink << " "
//							if (i == 0)
//								readable_file << "";
//							else
//								readable_file << " ";
//
//							readable_file << v[i * 8 + 0] << " "
//								<< v[i * 8 + 1] << " "
//								<< v[i * 8 + 2] << " "
//								<< v[i * 8 + 3] << " "
//								<< v[i * 8 + 4] << " "
//								<< v[i * 8 + 5] << " "
//								<< v[i * 8 + 6] << " "
//								<< v[i * 8 + 7];
//
//							if (i != data.peak_data.size() - 1)
//								readable_file << "\n";
//							else
//								readable_file << " ]";
//						}
//						//readable_file << " ]";
//
//						readable_file << "\n";
//					}
//				}
//			}
//			catch (const std::exception& e) {
//				std::cout << e.what();
//			}
//
//			ae.process(pusher, msg);
//			auto t2 = chrono::high_resolution_clock::now();
//			//cout << "\tsend time: " << (t2 - t1).count() << endl;
//		}
//	}
//	datasink.close();
//	context->stop();
//}