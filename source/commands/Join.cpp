#include "Server.hpp"

#include "Response.hpp"

using namespace std;

// Messaging / channel commands
void Server::Join(vector<string>& args, Client& client) {
	if (args.empty())
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "JOIN :Need more parameters"));

	vector<pair<string, string> > channels_pairs = SplitChannelsAndKeys(args);
	for (vector<pair<string, string> >::const_iterator pair =
	         channels_pairs.begin();
	     pair != channels_pairs.end(); ++pair) {
		if (!Channel::IsValidName(pair->first)) {
			QueueMessage(Response::Build(ERR_NOSUCHCHANNEL, client,
			                             pair->first + " :No such channel"));
			continue;
		}

		vector<Channel>::iterator it = GetChannelByName(pair->first);
		if (it != _channels.end()) {
			Channel& channel = *it;

			if (channel.HasClient(_socket_in_use))
				continue;

			if (channel.IsFull()) {
				QueueMessage(Response::Build(
				    ERR_CHANNELISFULL, client,
				    channel.GetName() + " :Cannot join channel (+l)"));
				continue;
			}

			if (channel.IsInviteOnly() && !channel.IsInvited(_socket_in_use)) {
				QueueMessage(Response::Build(
				    ERR_INVITEONLYCHAN, client,
				    channel.GetName() + " :Cannot join channel (+i)"));
				continue;
			}

			if (channel.HasKey() && !channel.CheckKey(pair->second)) {
				QueueMessage(Response::Build(
				    ERR_BADCHANNELKEY, client,
				    channel.GetName() + " :Cannot join channel (+k)"));
				continue;
			}

			channel.AddClient(_socket_in_use);
			channel.RemoveInvitation(_socket_in_use);

			BroadcastMessages(
			    channel.GetClients(),
			    Response::Build(client, "JOIN", channel.GetName()));

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

		QueueMessage(Response::Build(client, "JOIN", pair->first));

		SendChannelNames(_channels.back(), client);
	}
}

// Replies / notifications
void Server::SendChannelNames(Channel& channel, Client& client) {
	const vector<int>& members = channel.GetClients();
	string names;

	for (size_t i = 0; i < members.size(); ++i) {
		ClientMap::iterator it = _clients.find(members[i]);

		if (it == _clients.end())
			continue;

		if (!names.empty())
			names += ' ';

		if (channel.IsOperator(members[i]))
			names += '@';

		names += it->second.GetNickname();
	}

	QueueMessage(Response::Build(RPL_NAMREPLY, client,
	                             "= " + channel.GetName() + " :" + names));

	QueueMessage(Response::Build(RPL_ENDOFNAMES, client,
	                             channel.GetName() + " :End of /NAMES list"));
}
