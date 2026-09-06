#include "Constants.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Privmsg(std::vector<std::string> &args, Client &client) {
	if (args.empty()) {
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_NORECIPIENT,
		                             "No recipient given (PRIVMSG)"));
		return;
	}

	if (args.size() < 2 || args[1].empty()) {
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_NOTEXTTOSEND,
		                             "No text to send"));
		return;
	}

	for (ClientMap::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second.GetNickname() == args[0]) {
			const string message = ":" + client.GetNickname() + " PRIVMSG " +
			                       args[0] + " :" + args[1] + CR_LF;
			QueueMessage(it->first, message);
			return;
		}
	}

	QueueMessage(_socket_in_use,
	             Response::Build(StatusCode::ERR_NOSUCHNICK + " " + args[0],
	                             "No such nick/channel"));
}

