#include "../include/server.h"

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

#include <map>

static inline bool send(zmq::socket_t& socket, const std::string& message) {

	zmq::message_t zmq_msg(message.size());
	memcpy((void *)zmq_msg.data(), message.data(), message.size());
	return socket.send(zmq_msg);
}

static inline bool send_more(zmq::socket_t& socket, const std::string& message) {
	
	zmq::message_t zmq_msg(message.size());
	memcpy((void *)zmq_msg.data(), message.data(), message.size());
	return socket.send(zmq_msg, ZMQ_SNDMORE);
}

void Server::respond(const std::string& client, const std::string& response) {
	// addr frame
	send_more(router, client);
	// null frame
	send_more(router, "");
	// message frame
	send(router, response);
}

void Server::respond_more(const std::string& client, const std::vector<std::string>& responses) {
	
	if (responses.size() == 0)
		return;
	if (responses.size() == 1) {
		this->respond(client, responses[0]);
		return;
	}

	send_more(router, client);
	send_more(router, "");

	for (auto& response = responses.begin(); response != std::prev(responses.end()); response++)
	{
		send_more(router, *response);
	}

	send(router, responses.back());
}

std::tuple<std::string, std::vector<std::string>> Server::receive() {
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
		//std::cout << "Polling ..." << std::endl;
		zmq::poll(&items[0], 1, 1);

		if (!(items[0].revents & ZMQ_POLLIN))
			continue;

		std::string id;
		std::vector<std::string> msgs;
		std::tie(id, msgs) = receive();

		if (msgs.size() <= 0)
			continue;

		if (message_handler != NULL)
			message_handler(ReceivedRequest(*this, id, msgs));
	}
}

void Server::stop() {
	should_run = false;
}

void Server::register_handler(std::function<void(const ReceivedRequest)> handler)
{
	message_handler = handler;
}

std::shared_ptr<Server::Publisher> Server::get_publisher(std::string address)
{
	auto publisher = publishers[address].lock();
	if (!publisher)
	{
		zmq::socket_t sock(context, ZMQ_PUB);
		sock.bind(address);
		publishers[address] = publisher = std::make_shared<Server::Publisher>(std::move(sock), address);
	}
	return publisher;
}
