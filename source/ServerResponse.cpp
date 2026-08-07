#include "ServerResponse.hpp"

void ServerResponse::NotEnoughArgument(int socket, Client& client) {
	const std::string message = "caca";
	std::cout << "not enought";
	(void)socket;
	(void)client;
	(void)message;
}
