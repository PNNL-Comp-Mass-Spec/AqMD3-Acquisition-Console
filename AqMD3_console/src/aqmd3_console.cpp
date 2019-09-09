// FalkorAcquisition.cpp : Defines the entry point for the application.
//
//#include "UIMFWriter/UIMFWriter.h"

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

bool shouldExit;

int main() {
	shouldExit = false;

	zmq::context_t context(1);
	zmq::socket_t pub(context, ZMQ_PUB);
	pub.bind("tcp://*:6546");

	cout << "sleeping 1s" << endl;
	this_thread::sleep_for(1s);

	queue<AcquiredData> queue;
	mutex locker;
	condition_variable cond;
	cout << "start" << endl << endl;

	Digitizer digitizer("PXI3::0::0::INSTR", "Simulate=false, DriverSetup= Model=SA220P");
	std::shared_ptr<StreamingContext> dig_context = digitizer.configure_cst_zs1();

	thread ar[2];
	ar[0] = std::thread(publish_worker, std::ref(pub), std::ref(cond), std::ref(locker), std::ref(queue), dig_context);
	ar[1] = std::thread(digitizer_worker, std::ref(cond), std::ref(queue), dig_context);

	for (int i = 0; i < 2; i++) {
		ar[i].join();
	}

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
	shouldExit = true;
}

static void publish_worker(zmq::socket_t &pusher, std::condition_variable &sig, std::mutex &lockable, queue<AcquiredData>& workQueue, shared_ptr<StreamingContext> context)
{

	int i = 0;

	while (!shouldExit)
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