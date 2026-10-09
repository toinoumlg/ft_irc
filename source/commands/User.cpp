#include "Server.hpp"

void Server::User(vector<string> &args, Client &client) {
	if (!client.HasSetPassword())
		return QueueMessage(_socket_in_use,
		                    Response::Build(StatusCode::ERR_NOTREGISTERED,
		                                    client.GetNickname() + " :You have not registered"));

	if (args.size() < 4)
		return QueueMessage(
		    _socket_in_use,
		    Response::Build(StatusCode::ERR_NEEDMOREPARAMS + " USER",
		                    ":Need more parameters"));
	if (client.IsRegistered())
		return QueueMessage(
		    _socket_in_use,
		    Response::Build(StatusCode::ERR_ALREADYREGISTRED,
		                    ":Unauthorized command (already registered)"));

	client.SetUser(args);
	if (!client.IsRegistered() && client.SetRegistered())
		NotifyWelcome();
}
