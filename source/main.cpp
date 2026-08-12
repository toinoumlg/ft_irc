#include "Server.hpp"

int main(int ac, char *av[]) {
	if (ac != 3)
		return 1;
	try {
		Server irc = Server(av[1], av[2]);

		irc.Run();
	} catch (std::invalid_argument &error) {
	} catch (std::runtime_error &error) {
	}
}
