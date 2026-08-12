#include "Irc.hpp"

void Irc::Quit(std::vector<std::string> &, Client &) {
	throw ClientClose("user asked for disconnect");
}