#include "Response.hpp"
#include "Server.hpp"
#include "StatusCode.hpp"

void Server::Pass(std::vector<std::string> &args, Client &client) {
	if (args.empty())
		return QueueMessage(
		    _socket_in_use,
		    Response::Build(StatusCode::ERR_NEEDMOREPARAMS + " PASS",
		                    ":Need more parameters"));
	if (args[0] != _password)
		throw Client::Disconnect();
	if (client.IsRegistered())
		return QueueMessage(
		    Response::Build(StatusCode::ERR_ALREADYREGISTRED,
		                    ":Unauthorized command (already registered)"));
	client.SetPassword(true);
}
