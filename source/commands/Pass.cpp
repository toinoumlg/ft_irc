#include "Irc.hpp"
#include "Constants.hpp"
#include "ServerResponse.hpp"
#include "StatusCode.hpp"

void Irc::Pass(std::vector<std::string> &args, Client &client) {
	if (args.empty()) {
		ServerResponse::Send(_socket_in_use,
		                     StatusCode::ERR_NEEDMOREPARAMS + " PASS",
		                     "Need more parameters");
	} else if (args[0] != _password) {
		ServerResponse::Send(_socket_in_use, StatusCode::ERR_PASSWDMISMATCH,
		                     "Password incorrect");
	} else if (client.HasSetPassword()) {
		ServerResponse::Send(_socket_in_use, StatusCode::ERR_ALREADYREGISTRED,
		                     "Unauthorized command (already registered)");
	} else {
		client.SetPassword(true);
	}
}