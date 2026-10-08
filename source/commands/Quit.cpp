#include "Server.hpp"

void Server::Quit(vector<string>& args, Client&) {
	string reason = "Client Quit";

	if (!args.empty() && !args[0].empty())
		reason = args[0];

	NotifyQuit(_socket_in_use, reason);
	RemoveClientFromChannels(_socket_in_use);

	throw Client::Close();
}