#ifndef SERVER_H
#define SERVER_H

#include <zmq.hpp>

#include <mutex>
#include <tuple>
#include <string>
#include <memory>

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

private:
	zmq::context_t context;
	bool should_run;
	std::string router_addr;
	zmq::socket_t router;

	std::function<void(ReceivedRequest)> message_handler;

	std::tuple<std::string, std::vector<std::string>> receive();
	void respond(const std::string& client, const std::string& response);
	void respond_more(const std::string& client, const std::vector<std::string>& responses);

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
};
#endif // !SERVER_H
