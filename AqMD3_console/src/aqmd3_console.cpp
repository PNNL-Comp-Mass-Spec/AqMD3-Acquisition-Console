// FalkorAcquisition.cpp : Defines the entry point for the application.


#include "../include/acquisitionbufferpool.h"
#include "../include/server.h"
#include "../include/digitizer.h"
#include "../include/acquisitioncontext.h"
#include "../include/dgtcontext.h"
#include "../include/acquireddata.h"

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

#include <queue>

using namespace std;

static void digitizer_worker(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, shared_ptr<StreamingContext> context);
static void publish_worker(zmq::socket_t &pusher, std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, shared_ptr<StreamingContext> context);

bool should_exit = false;

int main() {
	auto server = new Server("tcp://*:5555");

	server->register_handler([&](Server::ReceivedRequest req) 
	{
		for (auto& payload = req.payload.begin(); payload != req.payload.end(); payload++)
		{
			if (*payload == "num instruments")
			{
				continue;
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
				vector<string> to_send(2);

				TofWidthMessage tofMsg;
				tofMsg.set_num_samples(100000);
				tofMsg.set_pusher_pulse_width(94016);

				to_send[0] = (tofMsg.SerializeAsString());

				vector<uint8_t> hash(picosha2::k_digest_size);
				picosha2::hash256(to_send[0].begin(), to_send[0].end(), hash.begin(), hash.end());

				to_send[1] = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
				req.send_responses(to_send);
			}

			if (*payload == "tof width")
			{
				continue;
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

		return; 
	});

	thread t([&] { server->run(); });

	getchar();
	server->stop();
	t.join();

	return 0;
}

static void digitizer_worker(std::condition_variable &sig, queue<AcquiredData>& resultsQueue, shared_ptr<StreamingContext> context)
{
	int triggers = 256;

	context->start();
	for (int i = 0; i < 300; i++)
	{
		auto t1 = chrono::high_resolution_clock::now();
		auto data = context->acquire(256, std::chrono::milliseconds(100));
		resultsQueue.push(data);
		auto t2 = chrono::high_resolution_clock::now();
		cout << "\tacquire time: " << (t2 - t1).count() << endl;
		sig.notify_one();
	}
	should_exit = true;
}

static void publish_worker(zmq::socket_t &pusher, std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, shared_ptr<StreamingContext> context)
{

	int i = 0;

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
			cout << "\tMessage: " << ++i << endl;
			ae.process(pusher, msg);
			auto t2 = chrono::high_resolution_clock::now();
			cout << "\tsend time: " << (t2 - t1).count() << endl;
		}
	}
	context->stop();
}