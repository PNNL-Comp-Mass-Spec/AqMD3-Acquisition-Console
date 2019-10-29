#ifndef SERVER_H
#define SERVER_H

#include <zmq.hpp>

#include <mutex>
#include <tuple>
#include <string>
#include <memory>
#include <map>

class Server {
public:
	class ReceivedRequest {
	private:
		std::string sender_id;
		Server& server;

	public:
		ReceivedRequest(Server& server, std::string sender, std::vector<std::string> payload)
			: server(server)
			, sender_id(sender)
			, payload(payload)
		{}

		std::vector<std::string> const payload;

		void send_response(const std::string& response) { server.respond(sender_id, response); }
		void send_responses(const std::vector<std::string>& responses) { server.respond_more(sender_id, responses); }
	};

	class Publisher {
	private:
		std::string connection;
		zmq::socket_t publisher_socket;
		std::mutex mut;

	public:
		Publisher(zmq::socket_t publisher_socket, std::string connection)
			: publisher_socket(std::move(publisher_socket))
			, connection(connection)
		{}

		void send(zmq::message_t& message, const std::string& subject)
		{
			std::lock_guard<std::mutex> guard(mut);

			zmq::message_t msg(subject.size());
			memcpy((void *)msg.data(), subject.data(), subject.size());

			publisher_socket.send(msg, ZMQ_SNDMORE);
			publisher_socket.send(message, 0);
		}
		
	};

private:
	zmq::context_t context;
	bool should_run;
	std::string router_addr;
	zmq::socket_t router;

	std::function<void(ReceivedRequest)> message_handler;

	std::tuple<std::string, std::vector<std::string>> receive();
	void respond(const std::string& client, const std::string& response);
	void respond_more(const std::string& client, const std::vector<std::string>& responses);

	std::map<std::string, std::weak_ptr<Publisher>> publishers;
public: 
	Server(std::string addr)
		: context(1)
		, should_run(true)
		, router_addr(addr)
		, router(context, ZMQ_ROUTER)
		, message_handler(NULL)
	{
		router.bind(addr.c_str());
	}

	void run();
	void stop();

	void register_handler(std::function<void(const ReceivedRequest)> handler);
	std::shared_ptr<Publisher> get_publisher(std::string address);

};
#endif // !SERVER_H
