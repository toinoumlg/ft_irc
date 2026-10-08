#include "Constants.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Topic(vector<string>& args, Client& client) {
	const string nickname =
	    client.GetNickname().empty() ? "*" : client.GetNickname();
	// Older error constants include a label; IRC replies need only the digits.

	if (!client.IsRegistered()) {
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_NOTREGISTERED,
		                             nickname + " :You have not registered"));
		return;
	}

	if (args.empty()) {
		QueueMessage(
		    _socket_in_use,
		    Response::Build(StatusCode::ERR_NEEDMOREPARAMS.substr(0, 3),
			                nickname + " TOPIC :Need more parameters"));
		return;
	}

	for (size_t i = 0; i < _channels.size(); ++i) {
		Channel& channel = _channels[i];
		if (channel.GetName() != args[0])
			continue;

		if (!channel.HasClient(_socket_in_use)) {
			QueueMessage(
			    _socket_in_use,
			    Response::Build(
			        StatusCode::ERR_NOTONCHANNEL.substr(0, 3),
			        nickname + " " + args[0] + " :You're not on that channel"));
			return;
		}

		if (args.size() == 1) {
			SendChannelTopic(channel, client);
			return;
		}

		if (channel.IsTopicRestricted() &&
		    !channel.IsOperator(_socket_in_use)) {
			QueueMessage(
			    _socket_in_use,
			    Response::Build(StatusCode::ERR_CHANOPRIVSNEEDED.substr(0, 3),
				                nickname + " " + args[0] +
				                    " :You're not channel operator"));
			return;
		}

		channel.SetTopic(args[1]);
		const string message = client.Identity() + " TOPIC " +
		                       channel.GetName() + " :" + channel.GetTopic() +
		                       CR_LF;
		const vector<int>& members = channel.GetClients();
		for (size_t j = 0; j < members.size(); ++j)
			QueueMessage(members[j], message);
		return;
	}

	QueueMessage(
	    _socket_in_use,
	    Response::Build(StatusCode::ERR_NOSUCHCHANNEL.substr(0, 3),
		                nickname + " " + args[0] + " :No such channel"));
}

void Server::SendChannelTopic(Channel& channel, Client& client) {
	if (channel.GetTopic().empty()) {
		QueueMessage(
		    _socket_in_use,
		    Response::Build(StatusCode::RPL_NOTOPIC,
			                client.GetNickname() + " " + channel.GetName() +
			                    " :No topic is set"));
	} else {
		QueueMessage(
		    _socket_in_use,
		    Response::Build(StatusCode::RPL_TOPIC,
			                client.GetNickname() + " " + channel.GetName() +
			                    " :" + channel.GetTopic()));
	}
}
