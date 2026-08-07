#pragma once

#include "Client.hpp"

class ServerResponse {
   public:
	static void NotEnoughArgument(int socket, Client& client);

   private:
	ServerResponse() {}

	~ServerResponse() {}
};
