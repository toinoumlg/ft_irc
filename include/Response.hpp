#pragma once

#include <string>

class Client;

enum StatusCode {
	// Registration / commands
	RPL_WELCOME = 1,
	RPL_YOURHOST = 2,
	RPL_CREATED = 3,
	RPL_MYINFO = 4,
	ERR_NOSUCHSERVER = 402,
	ERR_UNKNOWNCOMMAND = 421,
	ERR_NEEDMOREPARAMS = 461,
	ERR_PASSWDMISMATCH = 464,
	ERR_ALREADYREGISTRED = 462,
	ERR_NOTREGISTERED = 451,

	// Nicknames / restrictions
	ERR_NONICKNAMEGIVEN = 431,
	ERR_ERRONEUSNICKNAME = 432,
	ERR_NICKCOLLISION = 436,
	ERR_RESTRICTED = 484,
	ERR_UNAVAILRESOURCE = 437,
	ERR_NICKNAMEINUSE = 433,

	// Messaging
	ERR_NORECIPIENT = 411,
	ERR_NOTEXTTOSEND = 412,
	ERR_NOTOPLEVEL = 413,
	ERR_WILDTOPLEVEL = 414,
	ERR_TOOMANYTARGETS = 407,
	ERR_NOSUCHNICK = 401,
	RPL_AWAY = 301,
	ERR_NOORIGIN = 409,

	// Channels / membership
	ERR_NOSUCHCHANNEL = 403,
	ERR_CANNOTSENDTOCHAN = 404,
	RPL_NAMREPLY = 353,
	RPL_ENDOFNAMES = 366,
	ERR_USERNOTINCHANNEL = 441,
	ERR_NOTONCHANNEL = 442,
	ERR_CHANOPRIVSNEEDED = 482,

	// Topic / invitations / modes
	RPL_NOTOPIC = 331,
	RPL_TOPIC = 332,
	RPL_INVITING = 341,
	ERR_USERONCHANNEL = 443,
	ERR_INVITEONLYCHAN = 473,
	RPL_CHANNELMODEIS = 324,
	ERR_UNKNOWNMODE = 472,
	ERR_BADCHANNELKEY = 475,
	ERR_CHANNELISFULL = 471
};

class Response {
   public:
	// Message formatting
	static std::string Build(const std::string& command);
	static std::string Build(const std::string& command,
	                         const std::string& message);
	static std::string Build(StatusCode code, const Client& client,
	                         const std::string& message);
	static std::string Build(const Client& from, const std::string& command,
	                         const std::string& message);
	static std::string BuildError(StatusCode code, const Client& client,
	                              const std::string& target,
	                              const std::string& description);
	static std::string BuildError(StatusCode code, const Client& client,
	                              const std::string& first,
	                              const std::string& second,
	                              const std::string& description);

	static std::string Finish(const std::string& line);

	// Wire format constants
	static const std::string LOCALHOST;
	static const std::string CR_LF;

   private:
	Response() {}

	~Response() {}
};
