#include "Irc.hpp"
#include "Client.hpp"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int ac, char *av[]) {
    if (ac != 3)
        return 1;
    try {
        Irc irc = Irc(av[1], av[2]);

        irc.Run();
    } catch (std::exception &e) {
    }
}
