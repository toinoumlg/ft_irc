#pragma once

#include "Client.hpp"

class Response {
   public:
	static void Send(int socket, const std::string& command);
	static void Send(int socket, const std::string& status_code,
	                 const std::string& message);
	static void SendPrivate(int socket, const string& command,
	                        const string& message, const string& from);
	static void SendFrom(int socket, const string& command,
	                     const string& message, const string& from);

   private:
	Response() {}

	~Response() {}
};
