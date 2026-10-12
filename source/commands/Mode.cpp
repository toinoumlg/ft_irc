#include <iostream>
#include <string>

#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Mode(vector<string>& args, Client& client) {
	if (args.empty() || (args.size() > 1 && args[1].empty())) {
		QueueMessage(Response::Build(
		    StatusCode::ERR_NEEDMOREPARAMS,
		    client.GetNickname() + " MODE :Need more parameters"));
		return;
	}

	vector<Channel>::iterator it = GetChannelByName(args[0]);
	if (it == _channels.end())
		return QueueMessage(Response::Build(
		    StatusCode::ERR_NOSUCHCHANNEL,
		    client.GetNickname() + " " + args[0] + " :No such channel"));

	Channel& channel = *it;
	if (args.size() == 1) {
		QueueMessage(Response::Build(StatusCode::RPL_CHANNELMODEIS,
		                             client.GetNickname() + " " +
		                                 channel.GetName() + " " +
		                                 channel.GetModes()));
		return;
	}

	if (!channel.HasClient(_socket_in_use)) {
		QueueMessage(Response::Build(StatusCode::ERR_NOTONCHANNEL,
		                             client.GetNickname() + " " + args[0] +
		                                 " :You're not on that channel"));
		return;
	}

	if (!channel.IsOperator(_socket_in_use)) {
		QueueMessage(Response::Build(StatusCode::ERR_CHANOPRIVSNEEDED,
		                             client.GetNickname() + " " + args[0] +
		                                 " :You're not channel operator"));
		return;
	}

	bool enable = true;
	string changes;
	string params;
	for (string::const_iterator mode = args[1].begin(); mode != args[1].end();
	     ++mode) {
		switch (*mode) {
			case '+':
			case '-':
				if (!changes.empty() &&
				    (*changes.rbegin() == '-' || *changes.rbegin() == '+'))
					*changes.rbegin() = *mode;
				else
					changes += *mode;
				enable = *mode == '+';
				continue;
			case 'i':
				if (channel.IsInviteOnly() == enable)
					continue;
				channel.SetInviteOnly(enable);
				break;
			case 't':
				if (channel.IsTopicRestricted() == enable)
					continue;
				channel.SetTopicRestricted(enable);
				break;
			case 'k':
				if ((enable &&
				     (args.size() < 3 || channel.CheckKey(args[2]))) ||
				    (!enable && !channel.HasKey()))
					continue;
				if (!enable)
					channel.SetKey("");
				else {
					channel.SetKey(args[2]);
					params += " " + args[2];
					args.erase(args.begin() + 2);
				}
				break;
			case 'o': {
				if (args.size() < 3)
					continue;
				const int fd = IsUserInChannel(args[2], channel.GetClients());
				if (fd == -1) {
					QueueMessage(
					    Response::Build(StatusCode::ERR_USERNOTINCHANNEL,
					                    client.GetNickname() + ' ' + args[2] +
					                        ' ' + channel.GetName() + ' ' +
					                        ":They aren't on that channel"));
					continue;
				}
				if (enable)
					channel.AddOperator(fd);
				else
					channel.RemoveOperator(fd);
				params += " " + args[2];
				args.erase(args.begin() + 2);
				break;
			}
			default:
				QueueMessage(
				    Response::Build(StatusCode::ERR_UNKNOWNMODE,
				                    client.GetNickname() + " " + *mode +
				                        " :is unknown mode char to me for " +
				                        channel.GetName()));
				continue;
		}
		changes += *mode;
	}

	if (changes.size() > 1) {
		const string message =
		    Response::Build(client.Identity(), "MODE",
		                    channel.GetName() + " " + changes + params);
		BroadcastMessages(channel.GetClients(), message);
		channel.SetModes();
	}
}
