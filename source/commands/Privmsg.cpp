#include "Server.hpp"

#include "Response.hpp"

using namespace std;

// Messaging / channel commands
void Server::Privmsg(vector<string> &args, Client &client) {
	if (args.empty())
		return QueueMessage(Response::Build(ERR_NORECIPIENT, client,
		                                    ":No recipient given (PRIVMSG)"));

	if (args.size() < 2 || args[1].empty())
		return QueueMessage(
		    Response::Build(ERR_NOTEXTTOSEND, client, ":No text to send"));

	const vector<string> targets = SplitTargets(args[0]);
	for (vector<string>::const_iterator target = targets.begin();
	     target != targets.end(); ++target) {
		if (target->empty())
			continue;

		const vector<Channel>::const_iterator channel_it =
		    GetChannelByName(*target);
		if (channel_it != _channels.end()) {
			const Channel &channel = *channel_it;
			if (!channel.HasClient(_socket_in_use))
				QueueMessage(Response::Build(
				    ERR_CANNOTSENDTOCHAN, client,
				    channel.GetName() + " :Cannot send to channel"));
			else
				BroadcastMessages(
				    channel.GetClients(),
				    Response::Build(client, "PRIVMSG " + channel.GetName(),
				                    ":" + args[1]),
				    _socket_in_use);
			continue;
		}

		const int fd = GetClientByNick(*target);
		if (fd != -1) {
			QueueMessage(
			    fd,
			    Response::Build(client, "PRIVMSG " + _clients[fd].GetNickname(),
			                    ":" + args[1]));
			continue;
		}

		QueueMessage(Response::Build(ERR_NOSUCHNICK, client,
		                             *target + " :No such nick/channel"));
	}
}
