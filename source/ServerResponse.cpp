#include "ServerResponse.hpp"

static const std::string CR_LF = "\r\n";
static const std::string LOCALHOST = ":127.0.0.1";
// void ServerResponse::Send(int socket, std::string& command) {}

void ServerResponse::Send(const int socket, const std::string& status_code,
                          const std::string& message) {
	const std::string to_send =
	    LOCALHOST + " " + status_code + " :" + message + CR_LF;
	send(socket, to_send.c_str(), to_send.size(), 0);
}

void ServerResponse::Send(const int socket, const std::string& status_code,
                          const std::string& target,
                          const std::string& message) {
	(void)target;
	const std::string to_send = LOCALHOST + " " + status_code + " :" + message;
	send(socket, to_send.c_str(), to_send.size(), 0);
}
