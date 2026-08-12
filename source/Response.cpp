#include "Response.hpp"

static const string CR_LF = "\r\n";
static const string LOCALHOST = ":127.0.0.1";

void Response::Send(int socket, const string& command) {
	const string to_send = LOCALHOST + " " + command + CR_LF;
	send(socket, to_send.c_str(), to_send.size(), 0);
}

void Response::Send(const int socket, const string& status_code,
                    const string& message) {
	const string to_send =
	    LOCALHOST + " " + status_code + " " + message + CR_LF;
	send(socket, to_send.c_str(), to_send.size(), 0);
}

void Response::SendPrivate(const int socket, const string& command,
                           const string& message, const string& from) {
	const string to_send = ":" + from + " " + command + ":" + message + CR_LF;
	send(socket, to_send.c_str(), to_send.size(), 0);
}

void Response::SendFrom(const int socket, const string& command,
                        const string& message, const string& from) {
	const string to_send =
	    from + "@" + LOCALHOST + " " + command + " " + message + CR_LF;
	send(socket, to_send.c_str(), to_send.size(), 0);
}
