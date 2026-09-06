#pragma once

#include "Client.hpp"

class Response {
   public:

	static string Build(const string &command);
	static string Build(const string &status_code, const string &message);
	static string BuildPrivate(const string &command, const string &message,
							const string &from);
	static string BuildFrom(const string &command, const string &message,
							const string &from);

   private:
	Response() {}

	~Response() {}
};
