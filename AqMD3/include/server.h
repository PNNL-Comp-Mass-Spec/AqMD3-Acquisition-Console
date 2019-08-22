#ifndef SERVER_H
#define SERVER_H

#include <zmq.hpp>
#include <condition_variable>
#include <mutex>

class Server {
private:
	zmq::context_t context;
	bool should_run;

	void send_response(std::string msg);

public: 
	Server() : context(1),
		should_run(true)
	{}

	void run();
	void stop();
};

#endif // !SERVER_H
