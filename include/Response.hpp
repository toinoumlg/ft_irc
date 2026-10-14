#pragma once

#include "Client.hpp"

enum StatusCode {
	RPL_WELCOME = 1,
	ERR_UNKNOWNCOMMAND = 421,
	ERR_NEEDMOREPARAMS = 461,
	ERR_PASSWDMISMATCH = 464,
	ERR_ALREADYREGISTRED = 462,
	ERR_NONICKNAMEGIVEN = 431,
	ERR_ERRONEUSNICKNAME = 432,
	ERR_NICKCOLLISION = 436,
	ERR_RESTRICTED = 484,
	ERR_UNAVAILRESOURCE = 437,
	ERR_NICKNAMEINUSE = 433,
	ERR_NORECIPIENT = 411,
	ERR_NOTEXTTOSEND = 412,
	ERR_NOSUCHCHANNEL = 403,
	ERR_CANNOTSENDTOCHAN = 404,
	ERR_NOTOPLEVEL = 413,
	ERR_WILDTOPLEVEL = 414,
	ERR_TOOMANYTARGETS = 407,
	ERR_NOSUCHNICK = 401,
	RPL_AWAY = 301,
	RPL_NAMREPLY = 353,
	RPL_ENDOFNAMES = 366,
	ERR_USERNOTINCHANNEL = 441,
	ERR_NOTONCHANNEL = 442,
	ERR_CHANOPRIVSNEEDED = 482,
	ERR_BADCHANNELKEY = 475,
	RPL_NOTOPIC = 331,
	RPL_TOPIC = 332,
	ERR_NOTREGISTERED = 451,
	RPL_INVITING = 341,
	ERR_USERONCHANNEL = 443,
	ERR_INVITEONLYCHAN = 473,
	RPL_CHANNELMODEIS = 324,
	ERR_UNKNOWNMODE = 472,
	ERR_CHANNELISFULL = 471,
	ERR_NOORIGIN = 409
};

class Response {
   public:
	static string Build(const string &command);
	static string Build(const string &command, const string &message);
	static string Build(StatusCode code, const Client &client,
	                    const string &message);
	static string Build(const Client &from, const string &command,
	                    const string &message);

	static const string LOCALHOST;
	static const string CR_LF;

   private:
	Response() {}

	~Response() {}
};
