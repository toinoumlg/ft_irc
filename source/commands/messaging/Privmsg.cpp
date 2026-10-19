#include "Server.hpp"

#include "Response.hpp"

using namespace std;

// Messaging / channel commands
// [francais] Modification: résout les destinations privées complètes et autorise
// les messages extérieurs puisque le mode n n'est pas implémenté.
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
				BroadcastMessages(
				    channel.GetClients(),
				    Response::Build(client, "PRIVMSG " + channel.GetName(),
				                    ":" + args[1]),
				    _socket_in_use);
			continue;
		}

		const int fd = GetMessageTarget(*target);
		if (fd == -2) {
			QueueMessage(Response::BuildError(ERR_TOOMANYTARGETS, client, *target, "Duplicate recipients. No message delivered"));
			continue;
		}
		if (fd != -1) {
			QueueMessage(
			    fd,
			    Response::Build(client, "PRIVMSG " + _clients[fd].GetNickname(),
			                    ":" + args[1]));
			continue;
		}

		QueueMessage(Response::BuildError(ERR_NOSUCHNICK, client, *target, "No such nick/channel"));
	}
}
