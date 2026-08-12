#include "Server.hpp"
#include "Constants.hpp"
#include "Response.hpp"
#include "StatusCode.hpp"

void Server::Pass(std::vector<std::string> &args, Client &client) {
	if (args.empty())
		Response::Send(_socket_in_use, StatusCode::ERR_NEEDMOREPARAMS + " PASS",
					   ":Need more parameters");
	else if (args[0] != _password)
		Response::Send(_socket_in_use, StatusCode::ERR_PASSWDMISMATCH,
					   ":Password incorrect");
	else if (client.IsRegistered())
		Response::Send(_socket_in_use, StatusCode::ERR_ALREADYREGISTRED,
					   ":Unauthorized command (already registered)");
	else
		client.SetPassword(true);
}