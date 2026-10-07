#include "Constants.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Invite(vector<string>& args, Client& client) {
	const string nickname =
	    client.GetNickname().empty() ? "*" : client.GetNickname();

	if (!client.IsRegistered()) {
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_NOTREGISTERED,
		                             nickname + " :You have not registered"));
		return;
	}
	if (args.size() < 2) {
		QueueMessage(
		    _socket_in_use,
		    Response::Build(StatusCode::ERR_NEEDMOREPARAMS.substr(0, 3),
		                    nickname + " INVITE :Need more parameters"));
		return;
	}

	ClientMap::iterator target = _clients.begin();
	for (; target != _clients.end(); ++target) {
		if (target->second.GetNickname() == args[0] &&
		    target->second.IsRegistered())
			break;
	}
	if (target == _clients.end()) {
		QueueMessage(_socket_in_use,
		             Response::Build(
		                 StatusCode::ERR_NOSUCHNICK.substr(0, 3),
		                 nickname + " " + args[0] + " :No such nick/channel"));
		return;
	}

	for (size_t i = 0; i < _channels.size(); ++i) {
		Channel& channel = _channels[i];
		if (channel.GetName() != args[1])
			continue;

		if (!channel.HasClient(_socket_in_use)) {
			QueueMessage(
			    _socket_in_use,
			    Response::Build(
			        StatusCode::ERR_NOTONCHANNEL.substr(0, 3),
			        nickname + " " + args[1] + " :You're not on that channel"));
			return;
		}
		if (channel.IsInviteOnly() && !channel.IsOperator(_socket_in_use)) {
			QueueMessage(
			    _socket_in_use,
			    Response::Build(StatusCode::ERR_CHANOPRIVSNEEDED.substr(0, 3),
			                    nickname + " " + args[1] +
			                        " :You're not channel operator"));
			return;
		}
		if (channel.HasClient(target->first)) {
			QueueMessage(
			    _socket_in_use,
			    Response::Build(StatusCode::ERR_USERONCHANNEL,
			                    nickname + " " + args[0] + " " + args[1] +
			                        " :is already on channel"));
			return;
		}

		channel.AddInvitation(target->first);
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::RPL_INVITING,
		                             nickname + " " + args[0] + " " + args[1]));
		QueueMessage(target->first, client.Identity() + " INVITE " + args[0] +
		                                " :" + args[1] + CR_LF);
		return;
	}

	QueueMessage(
	    _socket_in_use,
	    Response::Build(StatusCode::ERR_NOSUCHCHANNEL.substr(0, 3),
	                    nickname + " " + args[1] + " :No such channel"));
}
