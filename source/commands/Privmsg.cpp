#include "Constants.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

// Not forget to add channel parsing  when added
void Server::Privmsg(std::vector<std::string>& args, Client& client) {
	if (args.empty()) {
		Response::Send(_socket_in_use, StatusCode::ERR_NORECIPIENT,
		               "No recipient given (PRIVMSG)");
	} else if (args.size() < 2) {
		Response::Send(_socket_in_use, StatusCode::ERR_NOTEXTTOSEND,
		               "No text to send");
	} else {
		for (ClientMap::iterator it = _clients.begin(); it != _clients.end();
		     ++it) {
			if (it->second.GetNickname() == args[0]) {
				std::string message =
				    ":" + client.GetNickname() + " PRIVMSG " + args[0] + " :";

				for (size_t i = 1; i < args.size() - 1; ++i)
					message += args[i] + " ";
				message += args.back();

				send(it->first, message.c_str(), message.size(), 0);
				return;
			}
		}
		Response::Send(_socket_in_use,
		               StatusCode::ERR_NOSUCHNICK + " " + args[0],
		               "No such nick/channel");
	}
}
