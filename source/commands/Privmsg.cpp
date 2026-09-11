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

	if (!args[0].empty() && args[0][0] == '#') {
		for (size_t i = 0; i < _channels.size(); ++i) {
			if (_channels[i].GetName() == args[0]) {

				if (!_channels[i].HasClient(_socket_in_use)) {
					QueueMessage(_socket_in_use,
								Response::Build(StatusCode::ERR_CANNOTSENDTOCHAN + " " + args[0],
												"Cannot send to channel"));
					return;
				}

				const string message =
					client.Identity() + " PRIVMSG " +
					args[0] + " :" + args[1] + CR_LF;

				const vector<int> &members = _channels[i].GetClients();

				for (size_t j = 0; j < members.size(); ++j) {
					if (members[j] != _socket_in_use)
						QueueMessage(members[j], message);
				}

				return;
			}
		}

		QueueMessage(_socket_in_use,
					Response::Build(StatusCode::ERR_NOSUCHCHANNEL + " " + args[0],
									"No such channel"));
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

