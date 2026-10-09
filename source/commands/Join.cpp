#include "Constants.hpp"
#include "Server.hpp"

void Server::Join(vector<string>& args, Client& client) {
	if (args.empty())
		return;

	const string& channel_name = args[0];

	if (!Channel::IsValidName(channel_name)) {
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_NOSUCHCHANNEL,
		                             channel_name + " :No such channel"));
		return;
	}

	Channel& channel = GetChannelByName(channel_name);
	if (&channel != &*_channels.end()) {
		if (channel.IsInviteOnly() && !channel.IsInvited(_socket_in_use)) {
			QueueMessage(
			    _socket_in_use,
			    Response::Build(StatusCode::ERR_INVITEONLYCHAN,
			                    client.GetNickname() + " " + channel_name +
			                        " :Cannot join channel (+i)"));
			return;
		}

		if (channel.HasClient(_socket_in_use))
			return;

		channel.AddClient(_socket_in_use);
		channel.RemoveInvitation(_socket_in_use);

		const vector<int>& members = channel.GetClients();

		for (size_t j = 0; j < members.size(); ++j)
			QueueMessage(members[j],
			             client.Identity() + " JOIN " + channel_name + CR_LF);

		if (!channel.GetTopic().empty())
			SendChannelTopic(channel, client);
		SendChannelNames(channel, client);
		return;
	}

	_channels.push_back(Channel(channel_name, _socket_in_use));

	QueueMessage(_socket_in_use,
	             client.Identity() + " JOIN " + channel_name + CR_LF);

	SendChannelNames(_channels.back(), client);
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
	    _socket_in_use,
	    Response::Build(StatusCode::RPL_NAMREPLY + " " + client.GetNickname(),
	                    "= " + channel.GetName() + " :" + names));

	QueueMessage(
	    _socket_in_use,
	    Response::Build(StatusCode::RPL_ENDOFNAMES + " " + client.GetNickname(),
	                    channel.GetName() + " :End of /NAMES list"));
}
