#include "StatusCode.hpp"

const string StatusCode::RPL_WELCOME = "001";

const string StatusCode::ERR_UNKNOWNCOMMAND = "421";

const string StatusCode::ERR_NEEDMOREPARAMS = "461";
const string StatusCode::ERR_PASSWDMISMATCH = "464";
const string StatusCode::ERR_ALREADYREGISTRED = "462";

const string StatusCode::ERR_NONICKNAMEGIVEN = "431";
const string StatusCode::ERR_ERRONEUSNICKNAME = "432";
const string StatusCode::ERR_NICKCOLLISION = "436";
const string StatusCode::ERR_RESTRICTED = "484";
const string StatusCode::ERR_UNAVAILRESOURCE = "437";
const string StatusCode::ERR_NICKNAMEINUSE = "433";

const string StatusCode::ERR_NORECIPIENT = "411";
const string StatusCode::ERR_NOTEXTTOSEND = "412";
const string StatusCode::ERR_NOSUCHCHANNEL = "403";
const string StatusCode::ERR_CANNOTSENDTOCHAN = "404";
const string StatusCode::ERR_NOTOPLEVEL = "413";
const string StatusCode::ERR_WILDTOPLEVEL = "414";
const string StatusCode::ERR_TOOMANYTARGETS = "407";
const string StatusCode::ERR_NOSUCHNICK = "401";
const string StatusCode::RPL_AWAY = "301";

const string StatusCode::RPL_NAMREPLY = "353";
const string StatusCode::RPL_ENDOFNAMES = "366";

const string StatusCode::ERR_USERNOTINCHANNEL = "441";
const string StatusCode::ERR_NOTONCHANNEL = "442";
const string StatusCode::ERR_CHANOPRIVSNEEDED = "482";

const string StatusCode::RPL_NOTOPIC = "331";
const string StatusCode::RPL_TOPIC = "332";
const string StatusCode::ERR_NOTREGISTERED = "451";

const string StatusCode::RPL_INVITING = "341";
const string StatusCode::ERR_USERONCHANNEL = "443";
const string StatusCode::ERR_INVITEONLYCHAN = "473";

const string StatusCode::RPL_CHANNELMODEIS = "324";
const string StatusCode::ERR_UNKNOWNMODE = "472";
