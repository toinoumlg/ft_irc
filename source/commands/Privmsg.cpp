#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Privmsg(std::vector<std::string> &args, Client &client) {
	if (args.empty()) {
		QueueMessage(Response::Build(StatusCode::ERR_NORECIPIENT,
		                             ":No recipient given (PRIVMSG)"));
		return;
	}

	const vector<Channel>::iterator it = GetChannelByName(args[0]);
	if (it != _channels.end()) {
		Channel &channel = *it;
		if (!channel.HasClient(_socket_in_use)) {
			QueueMessage(Response::Build(
			    StatusCode::ERR_CANNOTSENDTOCHAN + " " + args[0],
			    ":Cannot send to channel"));
			return;
		}

		BroadcastMessages(channel.GetClients(),
		                  Response::Build(client.Identity(),
		                                  "PRIVMSG " + args[0], ":" + args[1]),
		                  _socket_in_use);
		return;
	}

	for (ClientMap::iterator it = _clients.begin(); it != _clients.end();
	     ++it) {
		if (it->second.GetNickname() == args[0]) {
			QueueMessage(it->first,
			             Response::Build(client.Identity(),
			                             "PRIVMSG " + args[0], ":" + args[1]));
			return;
		}
	}

	QueueMessage(Response::Build(StatusCode::ERR_NOSUCHNICK + " " + args[0],
	                             ":No such nick/channel"));
}
