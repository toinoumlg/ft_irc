#include <algorithm>
#include <cctype>

#include "Helper.hpp"
#include "Response.hpp"
#include "Server.hpp"

void Server::Nick(vector<string> &args, Client &client) {
	if (!client.HasSetPassword())
		return QueueMessage(Response::Build(ERR_NOTREGISTERED, client,
		                                    ":You have not registered"));
	if (args.empty() || args[0].empty())
		return QueueMessage(
		    Response::Build(ERR_NONICKNAMEGIVEN, client, ":No nickname given"));
	if (!IsValidNickname(args[0]))
		return QueueMessage(Response::Build(ERR_ERRONEUSNICKNAME, client,
		                                    args[0] + " :Erroneous nickname"));
	if (args[0] == client.GetNickname())
		return;

	if (NicknameExists(args[0], client))
		return QueueMessage(Response::Build(
		    ERR_NICKNAMEINUSE, client, args[0] + " :Nickname already in use"));

	const Client old_client = client;
	client.SetNickname(args[0]);
	if (!client.IsRegistered() && client.SetRegistered())
		return NotifyWelcome();
	if (!client.IsRegistered())
		return;

	const string message =
	    Response::Build(old_client, "NICK", ":" + client.GetNickname());

	vector<int> notified;
	QueueMessage(message);
	notified.push_back(_socket_in_use);

	for (vector<Channel>::const_iterator channel = _channels.begin();
	     channel != _channels.end(); ++channel) {
		if (!channel->HasClient(_socket_in_use))
			continue;

		BroadcastMessages(channel->GetClients(), message, notified);
	}
}

bool Server::NicknameExists(const string &nickname, const Client &client) {
	const string lowerNickname = ToLower(nickname);
	for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end();
	     ++it) {
		if (&it->second == &client)
			continue;
		if (it->second.GetLower() == lowerNickname)
			return true;
	}
	return false;
}
