#include "Irc.hpp"
#include "Constants.hpp"
#include "ServerResponse.hpp"
#include "StatusCode.hpp"

void Irc::Privmsg(std::vector<std::string>& args, Client& client) {
	if (args.empty()) {
		ServerResponse::Send(_socket_in_use, StatusCode::ERR_NORECIPIENT,
		                     "No recipient given (PRIVMSG)");
	} else if (args.size() < 2) {
		ServerResponse::Send(_socket_in_use, StatusCode::ERR_NOTEXTTOSEND,
		                     "No text to send");
	} else {
		int target_fd = -1;
		for (ClientMap::iterator it = _clients.begin(); it != _clients.end();
		     ++it) {
			if (it->second.GetNickname() == args[0]) {
				target_fd = it->first;
				break;
			}
		}
		if (target_fd == -1) {
			ServerResponse::Send(_socket_in_use,
			                     StatusCode::ERR_NOSUCHNICK + " " + args[0],
			                     "No such nick/channel");
			return;
		}
		std::string message = ":" + client.GetNickname() + " PRIVMSG " +
		                      args[0] + " :" + args[1] + CR_LF;
		send(target_fd, message.c_str(), message.size(), 0);
	}
}