#include "Server.hpp"
#include "Constants.hpp"

void Server::Ping(vector<string> &, Client &) {
	std::string message = LOCALHOST + "PONG" + CR_LF;
	send(_socket_in_use, message.c_str(), message.size(), 0);
}