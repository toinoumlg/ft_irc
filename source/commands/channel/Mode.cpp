#include "Server.hpp"

#include <cstdlib>

#include "Helper.hpp"
#include "Response.hpp"

using namespace std;

// Messaging / channel commands
// [francais] Modification: parcourt tous les groupes de modes, borne les
// changements de parametres, adapte les consultations et valide les cles avant leur annonce.
void Server::Mode(vector<string>& args, Client& client) {
	if (args.empty() || (args.size() > 1 && args[1].empty()))
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client, "MODE :Need more parameters"));
	vector<Channel>::iterator it = GetChannelByName(args[0]);
	if (it == _channels.end())
		return QueueMessage(Response::BuildError(ERR_NOSUCHCHANNEL, client, args[0], "No such channel"));
	Channel& channel = *it;
	if (args.size() == 1)
		return QueueMessage(Response::Build(RPL_CHANNELMODEIS, client,
		    channel.GetName() + " " + channel.GetModes(channel.HasClient(_socket_in_use))));
	if (!channel.HasClient(_socket_in_use))
		return QueueMessage(Response::BuildError(ERR_NOTONCHANNEL, client, args[0], "You're not on that channel"));
	if (!channel.IsOperator(_socket_in_use))
		return QueueMessage(Response::BuildError(ERR_CHANOPRIVSNEEDED, client, args[0], "You're not channel operator"));

	bool enable = true;
	unsigned int parameter_changes = 0;
	// [francais] Chaque groupe de modes consomme ses propres paramètres avant
	// la lecture du groupe suivant. Une commande MODE applique au maximum
	// trois changements prenant un paramètre, tous groupes confondus.
	for (size_t group = 1; group < args.size();) {
		const string modes = args[group++];
		for (size_t j = 0; j < modes.size(); ++j) {
			const char mode = modes[j];
			if (mode == '+' || mode == '-') { enable = mode == '+'; continue; }
			if (string("itkol").find(mode) == string::npos) {
				QueueMessage(Response::BuildError(ERR_UNKNOWNMODE, client,
				    string(1, mode), "is unknown mode char to me for " + channel.GetName()));
				continue;
			}
			const bool takes_parameter = mode == 'k' || mode == 'o' || (mode == 'l' && enable);
			string parameter;
			if (takes_parameter) {
				if (group == args.size()) {
					QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client, "MODE :Need more parameters"));
					continue;
				}
				parameter = args[group++];
				if (parameter_changes == 3) continue;
			}
			bool changed = false;
			if (mode == 'i') {
				changed = channel.IsInviteOnly() != enable;
				channel.SetInviteOnly(enable);
			} else if (mode == 't') {
				changed = channel.IsTopicRestricted() != enable;
				channel.SetTopicRestricted(enable);
			} else if (mode == 'k') {
				if (!IsValidKey(parameter)) {
					QueueMessage(Response::BuildError(ERR_BADCHANNELKEY, client, channel.GetName(), "Invalid channel key"));
					continue;
				}
				changed = enable ? !channel.CheckKey(parameter) : channel.HasKey();
				channel.SetKey(enable ? parameter : "");
			} else if (mode == 'o') {
				const int fd = GetClientByNick(parameter);
				if (fd < 0) {
					QueueMessage(Response::BuildError(ERR_NOSUCHNICK, client, parameter, "No such nick/channel"));
					continue;
				}
				if (!channel.HasClient(fd)) {
					QueueMessage(Response::BuildError(ERR_USERNOTINCHANNEL, client, parameter, channel.GetName(), "They aren't on that channel"));
					continue;
				}
				changed = channel.IsOperator(fd) != enable;
				if (enable) channel.AddOperator(fd); else channel.RemoveOperator(fd);
				parameter = _clients[fd].GetNickname();
			} else if (mode == 'l') {
				unsigned short limit = 0;
				if (enable) {
					if (parameter.empty() || parameter.find_first_not_of("0123456789") != string::npos) continue;
					char* end = NULL;
					const long value = strtol(parameter.c_str(), &end, 10);
					if (*end || value < 1 || value > 65535) continue;
					limit = static_cast<unsigned short>(value);
				}
				changed = channel.Limit() != limit;
				channel.SetMaxClient(limit);
			}
			if (!changed) continue;
			if (takes_parameter) ++parameter_changes;
			// [francais] Une notification par changement évite toute ambiguïté
			// sur les paramètres,
			// notamment lorsqu'une clé commence par un deux-points.
			string message = channel.GetName() + " " + (enable ? "+" : "-") + mode;
			if (takes_parameter)
				message += (parameter.empty() || parameter[0] == ':' ? " :" : " ") + parameter;
			BroadcastMessages(channel.GetClients(), Response::Build(client, "MODE", message));
		}
	}
}
