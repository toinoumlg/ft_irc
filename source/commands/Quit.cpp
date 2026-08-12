#include "Server.hpp"

void Server::Quit(vector<string> &, Client &) {
	throw Client::Close();
}