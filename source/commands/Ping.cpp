#include "Server.hpp"

void Server::Ping(vector<string> &args, Client &client) {
	if (args.empty())
		return QueueMessage(
		    Response::Build(ERR_NOORIGIN, client, ":No origin specified"));
	QueueMessage(Response::Build("PONG", Response::LOCALHOST + " :" + args[0]));
}