#include <algorithm>

#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Kick(vector<string>& args, Client& client) {
	if (args.size() < 2) {
		QueueMessage(Response::Build(StatusCode::ERR_NEEDMOREPARAMS + " KICK",
		                             ":Need more parameters"));
		return;
	}

	const string& channel_name = args[0];
	const string& target_nick = args[1];

	vector<Channel>::iterator it = GetChannelByName(channel_name);
	if (it == _channels.end())
		return QueueMessage(
		    Response::Build(StatusCode::ERR_NOSUCHCHANNEL + " " + channel_name,
		                    ":No such channel"));

	Channel& channel = *it;
	if (!channel.HasClient(_socket_in_use)) {
		QueueMessage(
		    Response::Build(StatusCode::ERR_NOTONCHANNEL + " " + channel_name,
		                    ":You're not on that channel"));
		return;
	}

	if (!channel.IsOperator(_socket_in_use)) {
		QueueMessage(Response::Build(StatusCode::ERR_CHANOPRIVSNEEDED + ' ' +
		                                 client.GetNickname() + ' ' +
		                                 channel_name,
		                             ":You're not channel operator"));
		return;
	}

	int target_fd = -1;

	for (ClientMap::iterator it = _clients.begin(); it != _clients.end();
	     ++it) {
		if (it->second.GetNickname() == target_nick) {
			target_fd = it->first;
			break;
		}
	}

	if (target_fd == -1 || !channel.HasClient(target_fd)) {
		QueueMessage(Response::Build(StatusCode::ERR_USERNOTINCHANNEL + " " +
		                                 client.GetNickname() + " " +
		                                 channel_name,
		                             ":They aren't on that channel"));
		return;
	}

	string reason = client.GetNickname();

	if (args.size() >= 3 && !args[2].empty())
		reason = args[2];

	BroadcastMessages(
	    channel.GetClients(),
	    Response::Build(client.Identity(),
	                    "KICK " + channel.GetName() + " " + target_nick,
	                    ":" + reason));

	channel.RemoveClient(target_fd);

	if (channel.IsEmpty())
		_channels.erase(it);
}