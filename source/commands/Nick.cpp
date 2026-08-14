#include "Constants.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Nick(std::vector<std::string> &args, Client &client) {
	if (!client.HasSetPassword())
		Response::Send(_socket_in_use, StatusCode::ERR_RESTRICTED,
		               ":Your connection is restricted");
	else if (args.empty())
		Response::Send(_socket_in_use, StatusCode::ERR_NONICKNAMEGIVEN,
		               ":No nickname given");
	else if (HasInvalidChar(args[0]))
		Response::Send(_socket_in_use, StatusCode::ERR_ERRONEUSNICKNAME,
		               args[0] + " :Erroneous nickname");
	else if (NicknameExists(args[0]))
		Response::Send(_socket_in_use, StatusCode::ERR_NICKNAMEINUSE,
		               args[0] + " :Nickname already in use");
	else {
		const string old_nick = client.GetNickname();
		client.SetNickname(args[0]);
		// https://dd.ircdocs.horse/refs/commands/nick
		if (!old_nick.empty()) {
			const string from = ":" + old_nick + "!~" + client.GetUser();
			for (ClientMap::iterator it = _clients.begin();
			     it != _clients.end(); ++it)
				Response::SendFrom(it->first, "NICK",
				                   ":" + client.GetNickname(), from);
		}
	}
}

bool Server::NicknameExists(const string &nickname) {
	for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it)
		if (it->second.GetNickname() == nickname)
			return true;

	return false;
}

// https://www.unrealircd.org/docwiki/index.php?title=Nick_Character_Sets&mobileaction=toggle_view_desktop#Important_notes
bool Server::HasInvalidChar(const string &str) {
	if (str.empty() || isdigit(str[0]) || str[0] == '-' || str[0] == '/')
		return true;

	for (size_t i = 0; i < str.size(); ++i) {
		const char c = str[i];
		if (!isalnum(c) && c != '[' && c != '\\' && c != ']' && c != '^' &&
		    c != '_' && c != '-' && c != '{' && c != '}' && c != '|' &&
		    c != '`')
			return true;
	}

	return false;
}
