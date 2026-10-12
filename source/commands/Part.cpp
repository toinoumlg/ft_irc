#include "Server.hpp"

void Server::Part(vector<string> &args, Client &client) {
	if (args.empty() || args[0].empty())
		return QueueMessage(Response::Build(
		    StatusCode::ERR_NEEDMOREPARAMS,
		    client.GetNickname() + " PART :Need more parameters"));
	vector<string> names = SplitChannels(args[0]);
	const string part_message =
	    args.size() == 1 ? ":left the channel" : ":" + args[1];

	for (vector<string>::const_iterator name = names.begin();
	     name != names.end(); ++name) {
		vector<Channel>::iterator it = GetChannelByName(*name);
		if (it == _channels.end()) {
			QueueMessage(Response::Build(StatusCode::ERR_NOSUCHCHANNEL,
			                             *name + " :No such channel"));
			continue;
		}

		Channel &channel = *it;
		if (!channel.HasClient(_socket_in_use)) {
			QueueMessage(Response::Build(StatusCode::ERR_NOTONCHANNEL,
			                             client.GetNickname() + " " + *name +
			                                 " :You're not on that channel"));

			continue;
		}

		BroadcastMessages(
		    channel.GetClients(),
		    Response::Build(client.Identity(), "PART " + *name, part_message));

		channel.RemoveClient(_socket_in_use);
		if (channel.IsEmpty())
			_channels.erase(it);
	}
}