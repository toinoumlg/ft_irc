#include "Irc.hpp"

#include <cassert>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <unistd.h>

const Irc::CommandEntry Irc::_commands[] = {
    {"NICK", "nick", &Irc::Nick},
    {"USER", "user", &Irc::User},
    {"QUIT", "quit", &Irc::Quit},
    {"CAP", "cap", &Irc::Cap}
};

const char BR_CR[] = "\r\n";

Irc::Irc(std::string port, std::string password) : _socket_in_use(0),
                                                   _address() {
    _port = port;
    _password = password;
    int opt = 1;
    // socket creation
    // AF_INET = ipv4
    // SOCK_STREAM = TCP and SOCK_NONBLOCK = I/O non-blocking
    // protocol 0 is to target ips
    _server_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (_server_socket < 0)
        throw std::logic_error(strerror(errno));

    if (setsockopt(_server_socket,SOL_SOCKET,SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)))
        throw std::logic_error(strerror(errno));

    // Settings for incoming connection from client
    // Acceptation ipv4 AF_INET
    // From any address INADDR_ANY
    // Only on one defined port => htons(atoi(port))

    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(std::strtol(port.c_str(), NULL, 10));

    // apply the settings on our server socket (bind())
    if (bind(_server_socket, reinterpret_cast<sockaddr *>(&_address),
             sizeof(_address)) < 0)
        throw std::logic_error(strerror(errno));
    _connected_clients = 0;

    // create the server pollfd description
    const pollfd server = {
        .fd = _server_socket,
        .events = POLLIN,
        .revents = 0,
    };

    _pollfds.push_back(server);
    _addrlen = sizeof(_address);

    // enable passive listening of our server with 3 inside the queue
    if (listen(_server_socket, 3))
        throw std::logic_error(strerror(errno));
}

const int MAX_LENGTH = 512;

void Irc::Run() {
    while (true) {
        const int res = poll(&_pollfds[0], _pollfds.size(), 300);

        if (!res)
            continue;

        if (_pollfds[0].revents & POLLIN)
            ConnectClient();


        for (int i = 0; i < _connected_clients; ++i) {
            try {
                const short client_events = _pollfds[i + 1].revents;
                _socket_in_use = _pollfds[i + 1].fd;

                if (client_events & (POLLERR | POLLHUP | POLLNVAL))
                    throw ClientClose("from client_events");

                if (client_events & POLLIN) {
                    HandleRecv();

                    Client &client = _clients[_socket_in_use];
                    while (client.HasPendingCommand())
                        RunCommand(client.CreateArgs());
                }
            } catch (ClientClose &reason) {
                std::cout << "Client " << _clients[_socket_in_use] <<
                        " disconnected reason: " << reason.what() <<
                        std::endl;
                close(_socket_in_use);
                _clients.erase(_clients.find(_socket_in_use));
                _pollfds.erase(_pollfds.begin() + i + 1);
                _connected_clients--;
            }
        }
    }
}

void Irc::ConnectClient() {
    _socket_in_use = accept(_server_socket, reinterpret_cast<sockaddr *>
                            (&_address), &_addrlen);

    const pollfd pollfd = {
        .fd = _socket_in_use,
        .events = POLLIN,
        .revents = 0,
    };

    _clients[_socket_in_use] = Client(_address);
    _pollfds.push_back(pollfd);
    _connected_clients++;
}

bool Irc::HandleRecv() {
    uint8_t recv_buffer[MAX_LENGTH + 1];
    const ssize_t recv_size = recv(_socket_in_use, recv_buffer,
                                   MAX_LENGTH,
                                   0);

    if (recv_size == 0)
        throw ClientClose("user asked for disconnect");

    if (recv_size < 0)
        throw ClientClose("failed recv");

    _clients[_socket_in_use].PushBuffer(recv_buffer, recv_size);
    return true;
}

void Irc::RunCommand(std::vector<std::string> args) {
    if (args.empty())
        return;

    const std::string cmd = args[0];
    args.erase(args.begin());
    int max = sizeof(_commands) / sizeof(_commands[0]);
    for (int i = 0; i < max; i++) {
        if (cmd == _commands[i].lower || cmd == _commands[i].upper) {
            (this->*_commands[i].handler)(args);
            return;
        }
    }

    std::cout << "Invalid command: " << cmd;
    for (size_t i = 0; i < args.size(); ++i) {
        std::cout << "[" << args[i] << "] ";
    }
    std::cout << std::endl;
}

void Irc::Quit(std::vector<std::string> &) {
    throw ClientClose("user asked for disconnect");
}

void Irc::Nick(std::vector<std::string> &) {
}

// https://ircv3.net/specs/extensions/capability-negotiation.html
void Irc::Cap(std::vector<std::string> &args) {
    std::cout << args[0] << std::endl;
    if (args.size() != 2)
        return;
    std::string send_buffer;
    if (args[0] == "LS")
        send_buffer = "CAP * LS :multi-prefix sasl\r\n";
    else if (args[0] == "REQ")
        send_buffer = "CAP * ACK multi-prefix\r\n";

    size_t send_size = send(_socket_in_use, send_buffer.c_str(),
                            send_buffer.length(), 0);
    (void) send_size;
}

void Irc::User(std::vector<std::string> &args) {
    // for (size_t i = 0; i < args.size(); ++i) {
    //     std::cout << args[i] << " ";
    // }
    // std::cout << std::endl;
    (void) args;
}

Irc::~Irc() {
}
