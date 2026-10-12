#include "Response.hpp"

const std::string LOCALHOST = "127.0.0.1";
const std::string CR_LF = "\r\n";

string Response::Build(const string &command) {
	return ':' + LOCALHOST + ' ' + command + CR_LF;
}

string Response::Build(const string &status_code, const string &message) {
	return ':' + LOCALHOST + ' ' + status_code + ' ' + message + CR_LF;
}

string Response::Build(const string &from, const string &command,
                       const string &message) {
	return ':' + from + '@' + LOCALHOST + ' ' + command + ' ' + message + CR_LF;
}
