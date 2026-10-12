#pragma once
#include <string>
using namespace std;

class StatusCode {
   public:
	// Response confirming registration
	static const string RPL_WELCOME;

	// Invalid command
	static const string ERR_UNKNOWNCOMMAND;

	// PASS related
	static const string ERR_NEEDMOREPARAMS;
	static const string ERR_PASSWDMISMATCH;
	static const string ERR_ALREADYREGISTRED;

	// NICK related
	static const string ERR_NONICKNAMEGIVEN;
	static const string ERR_ERRONEUSNICKNAME;
	// for server-server conflict on nickname
	static const string ERR_NICKCOLLISION;
	static const string ERR_RESTRICTED;
	static const string ERR_UNAVAILRESOURCE;
	static const string ERR_NICKNAMEINUSE;

	// PRIVMSG related
	static const string ERR_NORECIPIENT;
	static const string ERR_NOTEXTTOSEND;
	static const string ERR_NOSUCHCHANNEL;
	static const string ERR_CANNOTSENDTOCHAN;
	static const string ERR_NOTOPLEVEL;
	static const string ERR_WILDTOPLEVEL;
	static const string ERR_TOOMANYTARGETS;
	static const string ERR_NOSUCHNICK;
	static const string RPL_AWAY;

	// JOIN / NAMES related
	static const string RPL_NAMREPLY;
	static const string RPL_ENDOFNAMES;

	static const string ERR_USERNOTINCHANNEL;
	static const string ERR_NOTONCHANNEL;
	static const string ERR_CHANOPRIVSNEEDED;
	static const string ERR_BADCHANNELKEY;

	// TOPIC related
	static const string RPL_NOTOPIC;
	static const string RPL_TOPIC;
	static const string ERR_NOTREGISTERED;

	// INVITE related
	static const string RPL_INVITING;
	static const string ERR_USERONCHANNEL;
	static const string ERR_INVITEONLYCHAN;

	// Channel MODE related
	static const string RPL_CHANNELMODEIS;
	static const string ERR_UNKNOWNMODE;
	static const string ERR_CHANNELISFULL;
};
