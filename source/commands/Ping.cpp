#include "Irc.hpp"
#include "Constants.hpp"


void Irc::Ping(std::vector<std::string> &, Client &) {
	std::string message = LOCALHOST + "PONG" + CR_LF;
	send(_socket_in_use, message.c_str(), message.size(), 0);
}