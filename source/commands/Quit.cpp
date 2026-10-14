#include "Server.hpp"

void Server::Quit(vector<string>& args, Client& client) {
	const string reason = "Quit: " + (args.empty() ? string() : args[0]);
	QueueMessage(Response::Build("ERROR", ":Closing link: (" + reason + ')'));
	client.MarkClosing(reason);
}