#include "Constants.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Kick(vector<string>& args, Client& client) {
	if (args.size() < 2) {
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_NEEDMOREPARAMS + " KICK",
		                             ":Need more parameters"));
		return;
	}

	const string channel_name = args[0];
	const string target_nick = args[1];

	for (size_t i = 0; i < _channels.size(); ++i) {
		if (_channels[i].GetName() != channel_name)
			continue;

		if (!_channels[i].HasClient(_socket_in_use)) {
			QueueMessage(_socket_in_use,
			             Response::Build(
			                 StatusCode::ERR_NOTONCHANNEL + " " + channel_name,
			                 ":You're not on that channel"));
			return;
		}

		if (!_channels[i].IsOperator(_socket_in_use)) {
			QueueMessage(_socket_in_use,
			             Response::Build(StatusCode::ERR_CHANOPRIVSNEEDED +
			                                 " " + channel_name,
			                             ":You're not channel operator"));
			return;
		}

		int target_fd = -1;

		for (ClientMap::iterator it = _clients.begin(); it != _clients.end();
		     ++it) {
			if (it->second.GetNickname() == target_nick) {
				target_fd = it->first;
				break;
			}
		}

		if (target_fd == -1 || !_channels[i].HasClient(target_fd)) {
			QueueMessage(
			    _socket_in_use,
			    Response::Build(StatusCode::ERR_USERNOTINCHANNEL + " " +
				                    target_nick + " " + channel_name,
				                ":They aren't on that channel"));
			return;
		}

		string reason = client.GetNickname();

		if (args.size() >= 3 && !args[2].empty())
			reason = args[2];

		const string message = client.Identity() + " KICK " + channel_name +
		                       " " + target_nick + " :" + reason + CR_LF;

		const vector<int>& members = _channels[i].GetClients();

		for (size_t j = 0; j < members.size(); ++j)
			QueueMessage(members[j], message);

		_channels[i].RemoveClient(target_fd);

		if (_channels[i].IsEmpty())
			_channels.erase(_channels.begin() + i);

		return;
	}

	QueueMessage(_socket_in_use, Response::Build(StatusCode::ERR_NOSUCHCHANNEL +
	                                                 " " + channel_name,
	                                             ":No such channel"));
}