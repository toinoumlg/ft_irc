#pragma once

#include <map>
#include <sys/socket.h>
#include <string>
#include <vector>
#include <netinet/in.h>

#include "Channel.hpp"
#include "Client.hpp"
#include <poll.h>

typedef std::map<int, Client> ClientMap;

class ClientClose : public std::exception {
public:
    ClientClose(const char *msg) : _message(msg) {
    }

    const char *what() const throw() {
        return _message.c_str();
    }

    ~ClientClose() throw() {
    };

private:
    std::string _message;
};


class Irc {
public:
    Irc(std::string port, std::string password);


    void Run();

    ~Irc();

private:
    Irc();

    typedef void (Irc::*CommandHandler)(std::vector<std::string> &);

    struct CommandEntry {
        const char *upper;
        const char *lower;
        CommandHandler handler;
    };

    static const CommandEntry _commands[];

    void RunCommand(std::vector<std::string> args);

    void ConnectClient();

    bool HandleRecv();

    void Cap(std::vector<std::string> &);

    void User(std::vector<std::string> &args);

    void Nick(std::vector<std::string> &);

    void Quit(std::vector<std::string> &);

    int _server_socket;
    int _socket_in_use;
    int _connected_clients;
    std::string _port;
    std::string _password;
    sockaddr_in _address;
    socklen_t _addrlen;
    std::vector<Channel> _channels;
    std::vector<pollfd> _pollfds;
    ClientMap _clients;
};
