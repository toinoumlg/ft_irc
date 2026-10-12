#pragma once

#include "Client.hpp"

class Response {
   public:
	static string Build(const string &command);
	static string Build(const string &status_code, const string &message);
	static string Build(const string &from, const string &command,
	                    const string &message);

   private:
	Response() {}

	~Response() {}
};
