#include "Server.hpp"

void Server::Ping(vector<string> &, Client &) {
	QueueMessage(Response::Build("PONG"));
}