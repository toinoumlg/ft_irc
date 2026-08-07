#include "Irc.hpp"

int main(int ac, char *av[]) {
	if (ac != 3)
		return 1;
	try {
		Irc irc = Irc(av[1], av[2]);

		irc.Run();
	} catch (std::invalid_argument &error) {
	} catch (std::runtime_error &error) {
	}
}
