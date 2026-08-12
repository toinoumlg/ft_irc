#include "Irc.hpp"
#include "Constants.hpp"
#include "ServerResponse.hpp"
#include "StatusCode.hpp"

void Irc::Nick(std::vector<std::string> &args, Client &client) {
	if (args.empty()) {
			ServerResponse::Send(_socket_in_use, StatusCode::ERR_NONICKNAMEGIVEN, "No nickname given");
	}
	// else if () {
	// 	std::for_each()
	// }
	else {
		client.SetNickname(args[0]);
	}
}