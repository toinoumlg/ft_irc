#include "Server.hpp"

void Server::User(vector<string> &args, Client &client) {
	if (!client.HasSetPassword())
		Response::Send(_socket_in_use, StatusCode::ERR_RESTRICTED,
					   ":Your connection is restricted");
	else if (args.size() < 4)
		Response::Send(_socket_in_use, StatusCode::ERR_NEEDMOREPARAMS + " USER",
					   ":Need more parameters");
	else if (client.IsRegistered())
		Response::Send(_socket_in_use, StatusCode::ERR_ALREADYREGISTRED,
					   ":Unauthorized command (already registered)");
	else {
		client.SetUser(args);
		Response::Send(
			_socket_in_use, StatusCode::RPL_WELCOME,
			client.GetNickname() + " :Welcome to a random IRC server !");
		client.SetRegistered(true);
	}
}