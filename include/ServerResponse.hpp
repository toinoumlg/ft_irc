#pragma once

#include "Client.hpp"

class ServerResponse {
   public:
	// void Send(int socket, std::string& command);
	static void Send(int socket, const std::string& status_code,
	                 const std::string& message);
	static void Send(int socket, const std::string& status_code,
	                 const std::string& target, const std::string& message);

   private:
	ServerResponse() {}

	~ServerResponse() {}
};
