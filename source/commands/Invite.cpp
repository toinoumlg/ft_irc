#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Invite(vector<string>& args, Client& client) {
	if (args.size() < 2) {
		QueueMessage(Response::Build(
		    StatusCode::ERR_NEEDMOREPARAMS,
		    client.GetNickname() + " INVITE :Need more parameters"));
		return;
	}

	ClientMap::iterator target = _clients.begin();
	for (; target != _clients.end(); ++target) {
		if (target->second.GetNickname() == args[0] &&
		    target->second.IsRegistered())
			break;
	}
	if (target == _clients.end()) {
		QueueMessage(Response::Build(
		    StatusCode::ERR_NOSUCHNICK,
		    client.GetNickname() + " " + args[0] + " :No such nick/channel"));
		return;
	}

	vector<Channel>::iterator it = GetChannelByName(args[1]);
	if (it == _channels.end()) {
		return QueueMessage(Response::Build(
		    StatusCode::ERR_NOSUCHCHANNEL,
		    client.GetNickname() + " " + args[1] + " :No such channel"));
	}

	Channel& channel = *it;
	if (!channel.HasClient(_socket_in_use)) {
		QueueMessage(Response::Build(StatusCode::ERR_NOTONCHANNEL,
		                             client.GetNickname() + " " + args[1] +
		                                 " :You're not on that channel"));
		return;
	}
	if (channel.IsInviteOnly() && !channel.IsOperator(_socket_in_use)) {
		QueueMessage(Response::Build(StatusCode::ERR_CHANOPRIVSNEEDED,
		                             client.GetNickname() + " " + args[1] +
		                                 " :You're not channel operator"));
		return;
	}
	if (channel.HasClient(target->first)) {
		QueueMessage(Response::Build(StatusCode::ERR_USERONCHANNEL,
		                             client.GetNickname() + " " + args[0] +
		                                 " " + args[1] +
		                                 " :is already on channel"));
		return;
	}

	channel.AddInvitation(target->first);
	QueueMessage(
	    Response::Build(StatusCode::RPL_INVITING,
	                    client.GetNickname() + " " + args[0] + " " + args[1]));
	QueueMessage(
	    target->first,
	    Response::Build(client.Identity(), "INVITE " + args[0], ":" + args[1]));
}
