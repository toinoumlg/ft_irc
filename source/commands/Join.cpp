#include "Server.hpp"

void Server::Join(vector<string>& args, Client& client) {
	if (args.empty())
		return;

	map<string, string> channels_pairs = SplitChannelsAndKeys(args);

	for (map<string, string>::const_iterator pair = channels_pairs.begin();
	     pair != channels_pairs.end(); ++pair) {
		if (!Channel::IsValidName(pair->first)) {
			QueueMessage(Response::Build(StatusCode::ERR_NOSUCHCHANNEL,
			                             pair->first + " :No such channel"));
			continue;
		}

		vector<Channel>::iterator it = GetChannelByName(pair->first);
		if (it != _channels.end()) {
			Channel& channel = *it;

			if (channel.IsInviteOnly() && !channel.IsInvited(_socket_in_use)) {
				QueueMessage(Response::Build(StatusCode::ERR_INVITEONLYCHAN,
				                             client.GetNickname() + " " +
				                                 pair->first +
				                                 " :Cannot join channel (+i)"));
				continue;
			}

			if (channel.HasKey() && !channel.CheckKey(pair->second)) {
				QueueMessage(Response::Build(StatusCode::ERR_BADCHANNELKEY,
				                             client.GetNickname() + " " +
				                                 pair->first +
				                                 " :Cannot join channel (+k)"));
				continue;
			}

			if (channel.HasClient(_socket_in_use))
				continue;

			channel.AddClient(_socket_in_use);
			channel.RemoveInvitation(_socket_in_use);

			BroadcastMessages(
			    channel.GetClients(),
			    Response::Build(client.Identity(), "JOIN", pair->first));

			if (!channel.GetTopic().empty())
				SendChannelTopic(channel, client);
			SendChannelNames(channel, client);
			continue;
		}

		if (pair->second.empty())
			_channels.push_back(Channel(pair->first, _socket_in_use));
		else
			_channels.push_back(
			    Channel(pair->first, _socket_in_use, pair->second));

		QueueMessage(Response::Build(client.Identity(), "JOIN", pair->first));

		SendChannelNames(_channels.back(), client);
	}
}

void Server::SendChannelNames(Channel& channel, Client& client) {
	const vector<int>& members = channel.GetClients();
	string names;

	for (size_t i = 0; i < members.size(); ++i) {
		ClientMap::iterator it = _clients.find(members[i]);

		if (it == _clients.end())
			continue;

		if (!names.empty())
			names += " ";

		if (channel.IsOperator(members[i]))
			names += "@";

		names += it->second.GetNickname();
	}

	QueueMessage(
	    Response::Build(StatusCode::RPL_NAMREPLY + " " + client.GetNickname(),
	                    "= " + channel.GetName() + " :" + names));

	QueueMessage(
	    Response::Build(StatusCode::RPL_ENDOFNAMES + " " + client.GetNickname(),
	                    channel.GetName() + " :End of /NAMES list"));
}
