#include "Server.hpp"

#include "Response.hpp"
#include "Helper.hpp"

using namespace std;

// Messaging / channel commands
// [francais] Modification: oublie JOIN 0 comme des départs, validation noms
// et cles, et refus des types de canaux non implementes + et !.
void Server::Join(vector<string>& args, Client& client) {
	if (args.empty())
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "JOIN :Need more parameters"));

	if (args[0] == "0") {
		for (size_t i = 0; i < _channels.size();) {
			if (!_channels[i].HasClient(_socket_in_use)) { ++i; continue; }
			vector<string> part(1, _channels[i].GetName());
			const size_t count = _channels.size();
			Part(part, client);
			if (_channels.size() == count) ++i;
		}
		return;
	}
	vector<pair<string, string> > channels_pairs = SplitChannelsAndKeys(args);
	for (vector<pair<string, string> >::const_iterator pair =
	         channels_pairs.begin();
	     pair != channels_pairs.end(); ++pair) {
		if (!Channel::IsValidName(pair->first)) {
			QueueMessage(Response::BuildError(ERR_NOSUCHCHANNEL, client, pair->first, "No such channel"));
			continue;
		}

		vector<Channel>::iterator it = GetChannelByName(pair->first);
		if (it != _channels.end()) {
			Channel& channel = *it;

			if (channel.HasClient(_socket_in_use))
				continue;

			if (channel.IsFull()) {
				QueueMessage(Response::BuildError(ERR_CHANNELISFULL, client, channel.GetName(), "Cannot join channel (+l)"));
				continue;
			}

			if (channel.IsInviteOnly() && !channel.IsInvited(_socket_in_use)) {
				QueueMessage(Response::BuildError(ERR_INVITEONLYCHAN, client, channel.GetName(), "Cannot join channel (+i)"));
				continue;
			}

			if (channel.HasKey() && !channel.CheckKey(pair->second)) {
				QueueMessage(Response::BuildError(ERR_BADCHANNELKEY, client, channel.GetName(), "Cannot join channel (+k)"));
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

		if (!pair->second.empty() && !IsValidKey(pair->second)) {
			QueueMessage(Response::BuildError(ERR_BADCHANNELKEY, client, pair->first, "Invalid channel key"));
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
// [francais] Modification: repartit les membres dans plusieurs reponses 353 de 512
// octets maximum sans couper ni perdre de pseudo.
void Server::SendChannelNames(Channel& channel, Client& client) {
	const vector<int>& members = channel.GetClients();
	const string prefix = "= " + channel.GetName() + " :";
	const size_t overhead = Response::Build(RPL_NAMREPLY, client, prefix).size();
	string names;
	for (size_t i = 0; i < members.size(); ++i) {
		ClientMap::iterator it = _clients.find(members[i]);
		if (it == _clients.end()) continue;
		const string nick = (channel.IsOperator(members[i]) ? "@" : "") + it->second.GetNickname();
		if (!names.empty() && overhead + names.size() + 1 + nick.size() > 512) {
			QueueMessage(Response::Build(RPL_NAMREPLY, client, prefix + names));
			names.clear();
		}
		if (!names.empty()) names += ' ';
		names += nick;
	}
	QueueMessage(Response::Build(RPL_NAMREPLY, client, prefix + names));
	QueueMessage(Response::Build(RPL_ENDOFNAMES, client,
	    channel.GetName() + " :End of /NAMES list"));
}
