#include <algorithm>

#include "Response.hpp"
#include "Server.hpp"

void Server::Kick(vector<string>& args, Client& client) {
	if (args.size() < 2)
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "KICK :Need more parameters"));

	const string& channel_name = args[0];
	const string& target_nick = args[1];

	vector<Channel>::iterator it = GetChannelByName(channel_name);
	if (it == _channels.end())
		return QueueMessage(Response::Build(
		    ERR_NOSUCHCHANNEL, client, channel_name + " :No such channel"));

	Channel& channel = *it;
	if (!channel.HasClient(_socket_in_use))
		return QueueMessage(
		    Response::Build(ERR_NOTONCHANNEL, client,
		                    channel_name + " :You're not on that channel"));

	if (!channel.IsOperator(_socket_in_use))
		return QueueMessage(
		    Response::Build(ERR_CHANOPRIVSNEEDED, client,
		                    channel_name + " :You're not channel operator"));

	const int target_fd = GetClientByNick(target_nick);

	if (target_fd == -1 || !channel.HasClient(target_fd))
		return QueueMessage(
		    Response::Build(ERR_USERNOTINCHANNEL, client,
		                    target_nick + ' ' + channel_name +
		                        " :They aren't on that channel"));

	string reason = client.GetNickname();

	if (args.size() >= 3 && !args[2].empty())
		reason = args[2];

	BroadcastMessages(channel.GetClients(),
	                  Response::Build(client,
	                                  "KICK " + channel.GetName() + " " +
	                                      _clients[target_fd].GetNickname(),
	                                  ":" + reason));

	channel.RemoveClient(target_fd);

	if (channel.IsEmpty())
		_channels.erase(it);
}