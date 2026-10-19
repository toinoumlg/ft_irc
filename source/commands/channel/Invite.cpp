#include "Server.hpp"

#include "Response.hpp"

using namespace std;

// Messaging / channel commands
// [francais] Modification: transmet aussi une invitation vers un canal inexistant sans
// le creer, au lieu de la refuser systematiquement.
void Server::Invite(vector<string>& args, Client& client) {
	if (args.size() < 2)
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "INVITE :Need more parameters"));

	const int target_fd = GetClientByNick(args[0]);

	if (target_fd == -1)
		return QueueMessage(Response::BuildError(ERR_NOSUCHNICK, client, args[0], "No such nick/channel"));

	vector<Channel>::iterator it = GetChannelByName(args[1]);
	if (it == _channels.end()) {
		QueueMessage(Response::Build(RPL_INVITING, client,
		    _clients[target_fd].GetNickname() + " :" + args[1]));
		QueueMessage(target_fd, Response::Build(client, "INVITE " +
		    _clients[target_fd].GetNickname(), ":" + args[1]));
		return;
	}

	Channel& channel = *it;
	if (!channel.HasClient(_socket_in_use))
		return QueueMessage(Response::BuildError(ERR_NOTONCHANNEL, client, channel.GetName(), "You're not on that channel"));
	if (channel.IsInviteOnly() && !channel.IsOperator(_socket_in_use))
		return QueueMessage(Response::BuildError(ERR_CHANOPRIVSNEEDED, client, channel.GetName(), "You're not channel operator"));
	if (channel.HasClient(target_fd))
		return QueueMessage(Response::BuildError(ERR_USERONCHANNEL, client, _clients[target_fd].GetNickname(), channel.GetName(), "is already on channel"));

	channel.AddInvitation(target_fd);
	QueueMessage(Response::Build(
	    RPL_INVITING, client,
	    _clients[target_fd].GetNickname() + " " + channel.GetName()));
	QueueMessage(
	    target_fd,
	    Response::Build(client, "INVITE " + _clients[target_fd].GetNickname(),
	                    ":" + channel.GetName()));
}
