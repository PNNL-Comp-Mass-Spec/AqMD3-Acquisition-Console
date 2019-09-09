#include "../include/server.h"

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

#include <map>

inline void Server::respond(std::string &client, std::string response) {
	/* begin response */
	// addr frame
	zmq::message_t client_addr_frame(client.size());
	memcpy((void *)client_addr_frame.data(), client.data(), client.size());
	router.send(client_addr_frame, ZMQ_SNDMORE);

	// null frame
	zmq::message_t null_frame(0);
	router.send(null_frame, ZMQ_SNDMORE);

	// message frame
	zmq::message_t response_frame(response.size());
	memcpy((void *)response_frame.data(), response.data(), response.size());
	router.send(response_frame);
}

inline std::tuple<std::string, std::vector<std::string>> Server::receive() {
	zmq::message_t message;
	std::vector<std::string> payload;

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
		payload.push_back(msg);
	} while (more);

	return std::make_tuple(client, payload);
}

void Server::run() 
{
	should_run = true;

	zmq::pollitem_t items[] = {
		{static_cast<void*>(router), 0, ZMQ_POLLIN, 0 }
	};

	while (should_run)
	{
		zmq::poll(&items[0], 1, 1);

		if (!(items[0].revents & ZMQ_POLLIN))
			continue;

		std::string id;
		std::vector<std::string> msgs;
		std::tie(id, msgs) = receive();

		if (msgs.size() <= 0)
			continue;

		if (message_handler != NULL)
			message_handler(ReceivedRequest(this, id, msgs));
	}
}

void Server::stop() {
	should_run = false;
}

void Server::register_hander(std::function<void(ReceivedRequest)> handler)
{
	message_handler = handler;
}
