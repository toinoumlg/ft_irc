#include "Server.hpp"

void Server::User(vector<string> &args, Client &client) {
	if (!client.HasSetPassword())
		return QueueMessage(Response::Build(ERR_NOTREGISTERED, client,
		                                    ":You have not registered"));

	if (args.size() < 4 || args[0].empty() || args[1].empty() ||
	    args[2].empty() || args[3].empty())
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "USER :Need more parameters"));
	if (client.IsRegistered())
		return QueueMessage(
		    Response::Build(ERR_ALREADYREGISTRED, client,
		                    ":Unauthorized command (already registered)"));

	client.SetUser(args);
	if (!client.IsRegistered() && client.SetRegistered())
		NotifyWelcome();
}
