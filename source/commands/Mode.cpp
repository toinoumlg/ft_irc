#include "Constants.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Mode(vector<string>& args, Client& client) {
	const string nickname =
	    client.GetNickname().empty() ? "*" : client.GetNickname();

	if (!client.IsRegistered()) {
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_NOTREGISTERED,
		                             nickname + " :You have not registered"));
		return;
	}
	if (args.empty() || (args.size() > 1 && args[1].empty())) {
		QueueMessage(
		    _socket_in_use,
		    Response::Build(StatusCode::ERR_NEEDMOREPARAMS.substr(0, 3),
		                    nickname + " MODE :Need more parameters"));
		return;
	}

	for (size_t i = 0; i < _channels.size(); ++i) {
		Channel& channel = _channels[i];
		if (channel.GetName() != args[0])
			continue;

		if (args.size() == 1) {
			string modes = "+";
			if (channel.IsInviteOnly())
				modes += "i";
			if (channel.IsTopicRestricted())
				modes += "t";
			QueueMessage(_socket_in_use,
			             Response::Build(
			                 StatusCode::RPL_CHANNELMODEIS,
			                 nickname + " " + channel.GetName() + " " + modes));
			return;
		}

		if (!channel.HasClient(_socket_in_use)) {
			QueueMessage(
			    _socket_in_use,
			    Response::Build(
			        StatusCode::ERR_NOTONCHANNEL.substr(0, 3),
			        nickname + " " + args[0] + " :You're not on that channel"));
			return;
		}
		if (!channel.IsOperator(_socket_in_use)) {
			QueueMessage(
			    _socket_in_use,
			    Response::Build(StatusCode::ERR_CHANOPRIVSNEEDED.substr(0, 3),
			                    nickname + " " + args[0] +
			                        " :You're not channel operator"));
			return;
		}

		bool enabled = true;
		char last_sign = '\0';
		string changes;
		for (size_t arg = 1; arg < args.size(); ++arg) {
			for (size_t j = 0; j < args[arg].size(); ++j) {
				const char mode = args[arg][j];
				if (mode == '+' || mode == '-') {
					enabled = mode == '+';
					continue;
				}

				bool current;
				if (mode == 'i')
					current = channel.IsInviteOnly();
				else if (mode == 't')
					current = channel.IsTopicRestricted();
				else {
					QueueMessage(_socket_in_use,
					             Response::Build(
					                 StatusCode::ERR_UNKNOWNMODE,
					                 nickname + " " + string(1, mode) +
					                     " :is unknown mode char to me for " +
					                     channel.GetName()));
					continue;
				}

				if (current == enabled)
					continue;
				if (mode == 'i')
					channel.SetInviteOnly(enabled);
				else
					channel.SetTopicRestricted(enabled);

				const char sign = enabled ? '+' : '-';
				if (sign != last_sign) {
					changes += sign;
					last_sign = sign;
				}
				changes += mode;
			}
		}

		if (!changes.empty()) {
			const string message = client.Identity() + " MODE " +
			                       channel.GetName() + " " + changes + CR_LF;
			const vector<int>& members = channel.GetClients();
			for (size_t j = 0; j < members.size(); ++j)
				QueueMessage(members[j], message);
		}
		return;
	}

	QueueMessage(
	    _socket_in_use,
	    Response::Build(StatusCode::ERR_NOSUCHCHANNEL.substr(0, 3),
	                    nickname + " " + args[0] + " :No such channel"));
}
