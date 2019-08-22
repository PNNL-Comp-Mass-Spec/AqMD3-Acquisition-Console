#include "../include/server.h"
#include "../include/digitizer.h"
#include "../include/acquisitionbufferpool.h"

#include <thread>

static void RespondAck(zmq::socket_t &router, std::string &client) {
	/* begin response */
	// addr frame
	zmq::message_t client_addr_frame(client.size());
	memcpy((void *)client_addr_frame.data(), client.data(), client.size());

	// null frame
	zmq::message_t null_frame(0);

	// message frame
	zmq::message_t ack_frame(3);
	memcpy((void *)ack_frame.data(), "ack", 3);
}

static std::tuple<std::string, std::vector<std::string>> GetResponse(zmq::socket_t &router) {
	zmq::message_t message;
	std::vector<std::string> vec;

	int more;
	size_t more_size = sizeof(more);

	/* begin receive */
	// get id
	router.recv(&message);
	std::string client = std::string(static_cast<char*>(message.data()), message.size());

	// null msg
	router.recv(&message);

	// payload
	do {
		router.recv(&message);
		router.getsockopt(ZMQ_RCVMORE, &more, &more_size);
		std::string msg = std::string(static_cast<char*>(message.data()), message.size());
		vec.push_back(msg);
	} while (more);

	return std::make_tuple(client, vec);
}

void Server::run() {
	should_run = true;
	Digitizer dig;

	zmq::socket_t router(context, ZMQ_ROUTER);
	zmq::socket_t publisher(context, ZMQ_PUB);

	std::string r_addr = "";
	std::string p_addr = "";

	router.bind(r_addr.c_str());
	publisher.bind(p_addr.c_str());

	zmq::pollitem_t items[] = {
		{static_cast<void*>(router), 0, ZMQ_POLLIN, 0 }
	};

	std::condition_variable sig;
	std::mutex lock;

	while (should_run)
	{
		zmq::poll(&items[0], 1, 1);

		if (!(items[0].revents & ZMQ_POLLIN))
			continue;

		std::string id;
		std::vector<std::string> msgs;
		std::tie(id, msgs) = GetResponse(router);
		RespondAck(router, id);

		if (msgs.size() <= 0)
			continue;

		if (msgs[0] == "acquire") {
			
		}
	}
}

void Server::stop() {
	should_run = false;
}

void digitizer_worker(zmq::socket_t &pusher, Digitizer &digitizer, std::condition_variable &sig, std::mutex &lockable) {
	std::unique_lock<std::mutex> lock(lockable);
	sig.wait(lock);

	int triggers = 5000;

	auto session = digitizer.configure();

	char sampleStreamName[] = "StreamCh1";
	char markerStreamName[] = "MarkersCh1";

	for (int i = 0; i < 10; i++)
	{
		//auto result = digitizer.acquire(
		//	session,
		//	*data,
		//	*markers,
		//	std::chrono::milliseconds(1),
		//	sampleStreamName,
		//	markerStreamName,
		//	10000);
	}
}