#include "Server.hpp"
#include "Constants.hpp"

void Server::Ping(vector<string> &, Client &) {
	std::string message = LOCALHOST + "PONG" + CR_LF;
	QueueMessage(_socket_in_use, message);
}