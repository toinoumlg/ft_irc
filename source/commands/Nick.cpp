#include <algorithm>
#include <cctype>

#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Nick(vector<string> &args, Client &client) {
	if (!client.HasSetPassword())
		return QueueMessage(_socket_in_use,
		                    Response::Build(StatusCode::ERR_NOTREGISTERED,
		                                    client.GetNickname() +
		                                        " :You have not registered"));
	if (args.empty())
		return QueueMessage(_socket_in_use,
		                    Response::Build(StatusCode::ERR_NONICKNAMEGIVEN,
		                                    ":No nickname given"));
	if (HasInvalidChar(args[0]))
		return QueueMessage(_socket_in_use,
		                    Response::Build(StatusCode::ERR_ERRONEUSNICKNAME,
		                                    args[0] + " :Erroneous nickname"));
	if (NicknameExists(args[0]))
		return QueueMessage(
		    _socket_in_use,
		    Response::Build(StatusCode::ERR_NICKNAMEINUSE,
		                    args[0] + " :Nickname already in use"));

	const string old_nick = client.GetNickname();
	const string old_ident = client.Identity();
	client.SetNickname(args[0]);
	if (!client.IsRegistered() && client.SetRegistered())
		return NotifyWelcome();
	if (!client.IsRegistered())
		return;

	const string message =
	    Response::Build(old_ident, "NICK", ":" + client.GetNickname());

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

bool Server::NicknameExists(const string &nickname) {
	for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end();
	     ++it)
		if (it->second.GetNickname() == nickname)
			return true;
	return false;
}

bool Server::HasInvalidChar(const string &str) {
	if (str.empty() || isdigit(static_cast<unsigned char>(str[0])) ||
	    str[0] == '-' || str[0] == '/')
		return true;

	for (size_t i = 0; i < str.size(); ++i) {
		const unsigned char c = static_cast<unsigned char>(str[i]);
		if (!isalnum(c) && c != '[' && c != '\\' && c != ']' && c != '^' &&
		    c != '_' && c != '-' && c != '{' && c != '}' && c != '|' &&
		    c != '`')
			return true;
	}
	return false;
}
