#include <csignal>

#include "Server.hpp"

int main(int ac, char* av[]) {
	if (ac != 3) {
		cerr << "Invalid arguments : \"./ircserv [port] [password]\"" << endl;
		return 1;
	}
	if (!Server::CheckPort(av[1])) {
		cerr << "Invalid port number must be between 1024 and 65535" << endl;
		return 1;
	}
	try {
		Server irc(av[1], av[2]);
		irc.Run();
	} catch (exception& e) {
		cerr << "Encountered an error : [" << e.what() << ']' << endl;
		return 1;
	}
	return 0;
}
