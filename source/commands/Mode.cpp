#include "Server.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

#include "Response.hpp"

using namespace std;

// Messaging / channel commands
void Server::Mode(vector<string>& args, Client& client) {
	if (args.empty() || (args.size() > 1 && args[1].empty()))
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "MODE :Need more parameters"));

	vector<Channel>::iterator it = GetChannelByName(args[0]);
	if (it == _channels.end())
		return QueueMessage(Response::Build(ERR_NOSUCHCHANNEL, client,
		                                    args[0] + " :No such channel"));

	Channel& channel = *it;
	if (args.size() == 1)
		return QueueMessage(
		    Response::Build(RPL_CHANNELMODEIS, client,
		                    channel.GetName() + " " + channel.GetModes()));

	if (!channel.HasClient(_socket_in_use))
		return QueueMessage(
		    Response::Build(ERR_NOTONCHANNEL, client,
		                    args[0] + " :You're not on that channel"));

	if (!channel.IsOperator(_socket_in_use))
		return QueueMessage(
		    Response::Build(ERR_CHANOPRIVSNEEDED, client,
		                    args[0] + " :You're not channel operator"));

	bool enable = true;
	string changes = "+";
	string params;
	for (string::const_iterator mode = args[1].begin(); mode != args[1].end();
	     ++mode) {
		switch (*mode) {
			case '+':
			case '-':
				if (*changes.rbegin() == '-' || *changes.rbegin() == '+')
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
				if (!enable) {
					if (args.size() >= 3)
						args.erase(args.begin() + 2);
					if (!channel.HasKey())
						continue;
					channel.SetKey("");
					params += " *";
					break;
				}
				if (args.size() < 3) {
					QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
					                             "MODE :Need more parameters"));
					continue;
				}
				channel.SetKey(args[2]);
				params += " " + args[2];
				args.erase(args.begin() + 2);
				break;
			case 'o': {
				if (args.size() < 3) {
					QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
					                             "MODE :Need more parameters"));
					continue;
				}
				const int fd = GetClientByNick(args[2]);

				if (fd == -1) {
					QueueMessage(
					    Response::Build(ERR_NOSUCHNICK, client,
					                    args[2] + " :No such nick/channel"));
					args.erase(args.begin() + 2);
					continue;
				}
				if (!channel.HasClient(fd)) {
					QueueMessage(Response::Build(
					    ERR_USERNOTINCHANNEL, client,
					    _clients[fd].GetNickname() + ' ' + channel.GetName() +
					        " :They aren't on that channel"));
					args.erase(args.begin() + 2);
					continue;
				}
				if (channel.IsOperator(fd) == enable) {
					args.erase(args.begin() + 2);
					continue;
				}
				if (enable)
					channel.AddOperator(fd);
				else
					channel.RemoveOperator(fd);
				params += " " + _clients[fd].GetNickname();
				args.erase(args.begin() + 2);
				break;
			}
			case 'l':
				if (enable) {
					if (args.size() < 3) {
						QueueMessage(
						    Response::Build(ERR_NEEDMOREPARAMS, client,
						                    "MODE :Need more parameters"));
						continue;
					}
					char* end = NULL;
					const long int value = strtol(args[2].c_str(), &end, 10);

					if (*end != '\0' || value < 1 || value > 65535 ||
					    value == channel.Limit()) {
						args.erase(args.begin() + 2);
						continue;
					}
					channel.SetMaxClient(static_cast<unsigned short>(value));
					params += ' ' + args[2];
					args.erase(args.begin() + 2);
				} else {
					if (!channel.Limit())
						continue;
					channel.SetMaxClient(0);
				}
				break;
			default:
				QueueMessage(Response::Build(
				    ERR_UNKNOWNMODE, client,
				    string(1, *mode) + " :is unknown mode char to me for " +
				        channel.GetName()));
				continue;
		}
		changes += *mode;
	}

	if (changes.size() > 1 &&
	    (*changes.rbegin() == '+' || *changes.rbegin() == '-')) {
		changes.erase(changes.size() - 1);
	}

	if (changes.size() > 1) {
		const string message = Response::Build(
		    client, "MODE", channel.GetName() + " " + changes + params);
		BroadcastMessages(channel.GetClients(), message);
		channel.SetModes();
	}
}
