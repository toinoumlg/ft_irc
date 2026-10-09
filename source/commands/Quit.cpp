#include "Server.hpp"

void Server::Quit(vector<string>& args, Client&) {
	string reason = "Client Quit";

	if (!args.empty() && !args[0].empty())
		reason = args[0];
	throw Client::Close();
}