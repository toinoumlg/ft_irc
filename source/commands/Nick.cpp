#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

#include <cctype>

void Server::Nick(std::vector<std::string> &args, Client &client) {
	if (!client.HasSetPassword())
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_RESTRICTED,
		                             ":Your connection is restricted"));
	else if (args.empty())
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_NONICKNAMEGIVEN,
		                             ":No nickname given"));
	else if (HasInvalidChar(args[0]))
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_ERRONEUSNICKNAME,
		                             args[0] + " :Erroneous nickname"));
	else if (NicknameExists(args[0]))
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_NICKNAMEINUSE,
		                             args[0] + " :Nickname already in use"));
	else {
		const string old_nick = client.GetNickname();
		client.SetNickname(args[0]);

		if (!old_nick.empty()) {
			const string from = ":" + old_nick;
			for (ClientMap::iterator it = _clients.begin(); it != _clients.end();
			     ++it)
				QueueMessage(it->first,
				             Response::BuildFrom("NICK",
				                                 ":" + client.GetNickname(), from));
		}
	}
}

bool Server::NicknameExists(const string &nickname) {
	for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it)
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
