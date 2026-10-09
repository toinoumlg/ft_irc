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
	client.SetNickname(args[0]);
	if (!client.IsRegistered() && client.SetRegistered())
		return NotifyWelcome();
	if (!client.IsRegistered())
		return;

	const string from = ":" + old_nick;
	const string message =
	    Response::BuildFrom("NICK", ":" + client.GetNickname(), from);

	vector<int> notified;
	QueueMessage(_socket_in_use, message);
	notified.push_back(_socket_in_use);

	for (vector<Channel>::const_iterator channel_it = _channels.begin();
	     channel_it != _channels.end(); ++channel_it) {
		if (!channel_it->HasClient(_socket_in_use))
			continue;

		const vector<int> &channelClients = channel_it->GetClients();

		for (vector<int>::const_iterator it = channelClients.begin();
		     it != channelClients.end(); ++it) {
			if (find(notified.begin(), notified.end(), *it) == notified.end()) {
				QueueMessage(*it, message);
				notified.push_back(*it);
			}
		}
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
