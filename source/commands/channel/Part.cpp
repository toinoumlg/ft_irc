#include "Server.hpp"

#include "Response.hpp"

using namespace std;

// Messaging / channel commands
// [francais] Modification: utilise le pseudo comme motif par défaut et supprime
// les canaux vides sans copie susceptible d'allouer.
void Server::Part(vector<string> &args, Client &client) {
	if (args.empty() || args[0].empty())
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "PART :Need more parameters"));
	vector<string> names = SplitTargets(args[0]);
	const string part_message =
	    args.size() == 1 ? ":" + client.GetNickname() : ":" + args[1];

	for (vector<string>::const_iterator name = names.begin();
	     name != names.end(); ++name) {
		vector<Channel>::iterator it = GetChannelByName(*name);
		if (it == _channels.end()) {
			QueueMessage(Response::BuildError(ERR_NOSUCHCHANNEL, client, *name, "No such channel"));
			continue;
		}

		Channel &channel = *it;
		if (!channel.HasClient(_socket_in_use)) {
			QueueMessage(Response::BuildError(ERR_NOTONCHANNEL, client, channel.GetName(), "You're not on that channel"));

			continue;
		}

		BroadcastMessages(
		    channel.GetClients(),
		    Response::Build(client, "PART " + channel.GetName(), part_message));

		channel.RemoveClient(_socket_in_use);
		if (channel.IsEmpty()) {
			channel.Swap(_channels.back());
			_channels.pop_back();
		}
	}
}
