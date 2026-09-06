#include "Response.hpp"

static const string CR_LF = "\r\n";
static const string LOCALHOST = ":127.0.0.1";

string Response::Build(const string &command) {
	return LOCALHOST + " " + command + CR_LF;
}

string Response::Build(const string &status_code, const string &message) {
	return LOCALHOST + " " + status_code + " " + message + CR_LF;
}

string Response::BuildPrivate(const string &command, const string &message,
                              const string &from) {
	return ":" + from + " " + command + " :" + message + CR_LF;
}

string Response::BuildFrom(const string &command, const string &message,
                           const string &from) {
	return from + " " + command + " " + message + CR_LF;
}
