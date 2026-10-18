#include "Server.hpp"

#include "Response.hpp"

using namespace std;

// Messaging / channel commands
void Server::Topic(vector<string>& args, Client& client) {
	if (args.empty())
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "TOPIC :Need more parameters"));

	vector<Channel>::iterator it = GetChannelByName(args[0]);
	if (it == _channels.end())
		return QueueMessage(Response::Build(ERR_NOSUCHCHANNEL, client,
		                                    args[0] + " :No such channel"));

	Channel& channel = *it;
	if (!channel.HasClient(_socket_in_use))
		return QueueMessage(
		    Response::Build(ERR_NOTONCHANNEL, client,
		                    args[0] + " :You're not on that channel"));

	if (args.size() == 1)
		return SendChannelTopic(channel, client);

	if (channel.IsTopicRestricted() && !channel.IsOperator(_socket_in_use))
		return QueueMessage(
		    Response::Build(ERR_CHANOPRIVSNEEDED, client,
		                    args[0] + " :You're not channel operator"));

	channel.SetTopic(args[1]);
	BroadcastMessages(channel.GetClients(),
	                  Response::Build(client, "TOPIC " + channel.GetName(),
	                                  ':' + channel.GetTopic()));
}

// Replies / notifications
void Server::SendChannelTopic(Channel& channel, Client& client) {
	if (channel.GetTopic().empty()) {
		QueueMessage(Response::Build(RPL_NOTOPIC, client,
		                             channel.GetName() + " :No topic is set"));
	} else {
		QueueMessage(Response::Build(
		    RPL_TOPIC, client, channel.GetName() + " :" + channel.GetTopic()));
	}
}
