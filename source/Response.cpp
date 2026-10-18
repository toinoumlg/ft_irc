#include "Response.hpp"

#include "Client.hpp"

using namespace std;

// Wire format constants
const string Response::CR_LF = "\r\n";
const string Response::LOCALHOST = "127.0.0.1";

// Message formatting
string Response::Build(const string& command) {
	return ':' + LOCALHOST + ' ' + command + CR_LF;
}

string Response::Build(const string& command, const string& message) {
	return ':' + LOCALHOST + ' ' + command + ' ' + message + CR_LF;
}

static string CodeToString(const StatusCode code) {
	char str[4];
	str[0] = static_cast<char>('0' + code / 100);
	str[1] = static_cast<char>('0' + code / 10 % 10);
	str[2] = static_cast<char>('0' + code % 10);
	str[3] = '\0';
	return str;
}

string Response::Build(const StatusCode code, const Client& client,
                       const string& message) {
	return ':' + LOCALHOST + ' ' + CodeToString(code) + ' ' +
	       client.GetNickname() + ' ' + message + CR_LF;
}

string Response::Build(const Client& from, const string& command,
                       const string& message) {
	return ':' + from.Identity() + '@' + LOCALHOST + ' ' + command + ' ' +
	       message + CR_LF;
}
