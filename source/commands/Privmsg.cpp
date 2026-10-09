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

	Channel &channel = GetChannelByName(args[0]);
	if (&channel != &*_channels.end() && channel.HasClient(_socket_in_use)) {
		if (!channel.HasClient(_socket_in_use)) {
			QueueMessage(_socket_in_use,
			             Response::Build(
			                 StatusCode::ERR_CANNOTSENDTOCHAN + " " + args[0],
			                 "Cannot send to channel"));
			return;
		}

		const string message =
		    client.Identity() + " PRIVMSG " + args[0] + " :" + args[1] + CR_LF;

		const vector<int> &members = channel.GetClients();

		for (size_t j = 0; j < members.size(); ++j) {
			if (members[j] != _socket_in_use) {
				QueueMessage(members[j], message);
			}
		}
		return;
	}

	for (ClientMap::iterator it = _clients.begin(); it != _clients.end();
	     ++it) {
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

