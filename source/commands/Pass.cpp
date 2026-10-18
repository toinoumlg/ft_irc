#include "Server.hpp"

#include "Response.hpp"

using namespace std;

// Connection commands
void Server::Pass(vector<string> &args, Client &client) {
	if (args.empty())
		return QueueMessage(Response::Build(ERR_NEEDMOREPARAMS, client,
		                                    "PASS :Need more parameters"));

	if (client.IsRegistered())
		return QueueMessage(
		    Response::Build(ERR_ALREADYREGISTRED, client,
		                    ":Unauthorized command (already registered)"));
	if (args[0] != _password) {
		QueueMessage(
		    Response::Build(ERR_PASSWDMISMATCH, client, ":Password incorrect"));
		QueueMessage(
		    Response::Build("ERROR", ":Closing link (password incorrect)"));
		return client.MarkClosing("");
	}

	client.SetPassword(true);
}
