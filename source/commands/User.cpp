#include "Irc.hpp"

void Irc::User(std::vector<std::string> &args, Client &client) {
	if (!client.IsRegistered() && !client.HasSetPassword()) {
		std::cout << "Client didn't set password yet" << std::endl;
		return;
	}

	client.SetUser(args[0]);
}