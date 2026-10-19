#include "Server.hpp"

#include "Response.hpp"

using namespace std;

// Replies / notifications
// [francais] Modification: envoie 001 a 004 avec l'identité
// complète, la version, la date et les modes au lieu de 001 seul.
void Server::NotifyWelcome() {
	const Client& client = _clients[_socket_in_use];
	string greeting = "Welcome to the Internet Relay Network ";
	const size_t overhead = Response::Build(RPL_WELCOME, client, ":").size();
	if (overhead + greeting.size() + client.Identity().size() > 512)
		greeting.clear();
	QueueMessage(Response::Build(RPL_WELCOME, client,
	                             ":" + greeting + client.Identity()));
	QueueMessage(Response::Build(
	    RPL_YOURHOST, client,
	    ":Your host is " + Response::LOCALHOST + ", running version ft_irc-1"));
	QueueMessage(Response::Build(RPL_CREATED, client,
	                             ":This server was created " + _created));
	QueueMessage(Response::Build(RPL_MYINFO, client,
	                             Response::LOCALHOST + " ft_irc-1 - itkol"));
}

void Server::NotifyQuit() {
	const Client& client = _clients[_socket_in_use];
	if (!client.IsRegistered())
		return;

	const string message =
	    Response::Build(client, "QUIT", ':' + client.QuitReason());

	vector<int> notified;
	notified.push_back(_socket_in_use);

	for (size_t i = 0; i < _channels.size(); ++i)
		if (_channels[i].HasClient(_socket_in_use))
			BroadcastMessages(_channels[i].GetClients(), message, notified);
}
