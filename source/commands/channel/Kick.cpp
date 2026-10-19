#include "Server.hpp"

#include <algorithm>

#include "Response.hpp"

using namespace std;

// Messaging / channel commands
// [francais] Modification: oublie de décomposer les listes de canaux et de personnes pour
// appliquer un KICK individuel à chaque association valide.
void Server::Kick(vector<string>& args, Client& client) {
	if (args.size() < 2)
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client, "KICK :Need more parameters"));
	vector<string> channels = SplitTargets(args[0]);
	vector<string> users = SplitTargets(args[1]);
	if (channels.empty() || users.empty() || (channels.size() != 1 && channels.size() != users.size()))
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client, "KICK :Invalid target lists"));
	for (size_t i = 0; i < users.size(); ++i) {
		vector<string> one;
		one.push_back(channels[channels.size() == 1 ? 0 : i]);
		one.push_back(users[i]);
		if (args.size() > 2) one.push_back(args[2]);
		KickOne(one, client);
	}
}

// [francais] Ajout par extraction: reprend les controles d'un KICK individuel
// pour traiter les listes et retire un canal vide sans copie allouante.
void Server::KickOne(vector<string>& args, Client& client) {
	if (args.size() < 2)
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "KICK :Need more parameters"));

	const string& channel_name = args[0];
	const string& target_nick = args[1];

	vector<Channel>::iterator it = GetChannelByName(channel_name);
	if (it == _channels.end())
		return QueueMessage(Response::BuildError(ERR_NOSUCHCHANNEL, client, channel_name, "No such channel"));

	Channel& channel = *it;
	if (!channel.HasClient(_socket_in_use))
		return QueueMessage(
		    Response::BuildError(ERR_NOTONCHANNEL, client, channel_name, "You're not on that channel"));

	if (!channel.IsOperator(_socket_in_use))
		return QueueMessage(
		    Response::BuildError(ERR_CHANOPRIVSNEEDED, client, channel_name, "You're not channel operator"));

	const int target_fd = GetClientByNick(target_nick);

	if (target_fd == -1 || !channel.HasClient(target_fd))
		return QueueMessage(
		    Response::BuildError(ERR_USERNOTINCHANNEL, client, target_nick, channel_name, "They aren't on that channel"));

	string reason = client.GetNickname();

	if (args.size() >= 3)
		reason = args[2];

	BroadcastMessages(channel.GetClients(),
	                  Response::Build(client,
	                                  "KICK " + channel.GetName() + " " +
	                                      _clients[target_fd].GetNickname(),
	                                  ":" + reason));

	channel.RemoveClient(target_fd);

	if (channel.IsEmpty()) {
		channel.Swap(_channels.back());
		_channels.pop_back();
	}
}
