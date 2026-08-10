#pragma once
#include <string>

class StatusCode {
   public:
	// PASS related ERR
	static const std::string ERR_NEEDMOREPARAMS;
	static const std::string ERR_PASSWDMISMATCH;
	static const std::string ERR_ALREADYREGISTRED;

	// NICK related ERR
	static const std::string ERR_NONICKNAMEGIVEN;
	static const std::string ERR_ERRONEUSNICKNAME;
	static const std::string ERR_NICKCOLLISION;
	static const std::string ERR_RESTRICTED;
	static const std::string ERR_UNAVAILRESOURCE;
	static const std::string ERR_NICKNAMEINUSE;
};





