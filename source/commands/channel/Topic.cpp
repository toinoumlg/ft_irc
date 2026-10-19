#include "Server.hpp"

#include "Response.hpp"

using namespace std;

// Messaging / channel commands
// [francais] Modification: mémorise seulement le texte qui tient dans l'annonce TOPIC.
void Server::Topic(vector<string>& args, Client& client) {
	if (args.empty())
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "TOPIC :Need more parameters"));

	vector<Channel>::iterator it = GetChannelByName(args[0]);
	if (it == _channels.end())
		return QueueMessage(Response::BuildError(ERR_NOSUCHCHANNEL, client, args[0], "No such channel"));

	Channel& channel = *it;
	if (!channel.HasClient(_socket_in_use))
		return QueueMessage(
		    Response::BuildError(ERR_NOTONCHANNEL, client, args[0], "You're not on that channel"));

	if (args.size() == 1)
		return SendChannelTopic(channel, client);

	if (channel.IsTopicRestricted() && !channel.IsOperator(_socket_in_use))
		return QueueMessage(
		    Response::BuildError(ERR_CHANOPRIVSNEEDED, client, args[0], "You're not channel operator"));

	const size_t announcement = Response::Build(client, "TOPIC " + channel.GetName(), ":").size();
	const size_t reply = Response::Build(RPL_TOPIC, client, channel.GetName() + " :").size()
	                     + 9 - client.GetNickname().size();
	const size_t overhead = announcement > reply ? announcement : reply;
	channel.SetTopic(args[1].substr(0, 512 - overhead));
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
