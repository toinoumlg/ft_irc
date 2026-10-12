#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Topic(vector<string>& args, Client& client) {
	if (args.empty()) {
		QueueMessage(Response::Build(
		    StatusCode::ERR_NEEDMOREPARAMS,
		    client.GetNickname() + " TOPIC :Need more parameters"));
		return;
	}

	vector<Channel>::iterator it = GetChannelByName(args[0]);
	if (it == _channels.end())
		return QueueMessage(Response::Build(
		    StatusCode::ERR_NOSUCHCHANNEL,
		    client.GetNickname() + " " + args[0] + " :No such channel"));

	Channel& channel = *it;
	if (!channel.HasClient(_socket_in_use)) {
		QueueMessage(Response::Build(StatusCode::ERR_NOTONCHANNEL,
		                             client.GetNickname() + " " + args[0] +
		                                 " :You're not on that channel"));
		return;
	}

	if (args.size() == 1)
		return SendChannelTopic(channel, client);

	if (channel.IsTopicRestricted() && !channel.IsOperator(_socket_in_use)) {
		QueueMessage(Response::Build(StatusCode::ERR_CHANOPRIVSNEEDED,
		                             client.GetNickname() + " " + args[0] +
		                                 " :You're not channel operator"));
		return;
	}

	channel.SetTopic(args[1]);
	BroadcastMessages(
	    channel.GetClients(),
	    Response::Build(client.Identity(), "TOPIC " + channel.GetName(),
	                    ':' + channel.GetTopic()));
}

void Server::SendChannelTopic(Channel& channel, Client& client) {
	if (channel.GetTopic().empty()) {
		QueueMessage(Response::Build(StatusCode::RPL_NOTOPIC,
		                             client.GetNickname() + " " +
		                                 channel.GetName() +
		                                 " :No topic is set"));
	} else {
		QueueMessage(Response::Build(StatusCode::RPL_TOPIC,
		                             client.GetNickname() + " " +
		                                 channel.GetName() + " :" +
		                                 channel.GetTopic()));
	}
}
