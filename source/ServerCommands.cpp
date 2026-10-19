#include "Server.hpp"

#include <cctype>

#include "Response.hpp"

using namespace std;

const Server::CommandEntry Server::_commands[] = {
    {"NICK", false, &Server::Nick},      {"USER", false, &Server::User},
    {"PASS", false, &Server::Pass},      {"CAP", false, &Server::Cap},
    {"QUIT", false, &Server::Quit},      {"PING", true, &Server::Ping},
    {"PRIVMSG", true, &Server::Privmsg}, {"JOIN", true, &Server::Join},
    {"KICK", true, &Server::Kick},       {"TOPIC", true, &Server::Topic},
    {"INVITE", true, &Server::Invite},   {"MODE", true, &Server::Mode},
    {"PART", true, &Server::Part}};

// Command dispatch
// [francais] Modification: convertit chaque octet en unsigned char avant
// toupper pour éviter les valeurs
// négatives interdites.
void Server::RunCommand(Client& client) {
	vector<string> args = client.CreateArgs();

	if (args.empty() || args[0].empty())
		return;

	string cmd;
	for (string::const_iterator it = args[0].begin(); it != args[0].end(); ++it)
		cmd += static_cast<char>(toupper(static_cast<unsigned char>(*it)));

	args.erase(args.begin());
	const int max = sizeof(_commands) / sizeof(_commands[0]);
	for (int i = 0; i < max; ++i)
		if (cmd == _commands[i].name) {
			if (_commands[i].needsRegistration && !client.IsRegistered())
				return QueueMessage(Response::Build(
				    ERR_NOTREGISTERED, client, ":You have not registered"));

			return (this->*_commands[i].handler)(args, client);
		}

	QueueMessage(
	    Response::BuildError(ERR_UNKNOWNCOMMAND, client, cmd, "Unknown command"));
}
