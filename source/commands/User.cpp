#include "Server.hpp"

void Server::User(vector<string> &args, Client &client) {
	if (!client.HasSetPassword())
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_RESTRICTED,
		                             ":Your connection is restricted"));
	else if (args.size() < 4)
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_NEEDMOREPARAMS + " USER",
		                             ":Need more parameters"));
	else if (client.IsRegistered())
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::ERR_ALREADYREGISTRED,
		                             ":Unauthorized command (already registered)"));
	else {
		client.SetUser(args);
		QueueMessage(_socket_in_use,
		             Response::Build(StatusCode::RPL_WELCOME,
		                             client.GetNickname() +
		                                 " :Welcome to a random IRC server !"));
		client.SetRegistered(true);
	}
}
