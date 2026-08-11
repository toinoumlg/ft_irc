#include "Irc.hpp"

#include "ServerResponse.hpp"
#include "StatusCode.hpp"

static const std::string LOCALHOST = ":localhost ";
static const std::string CR_LF = "\r\n";

const Irc::CommandEntry Irc::_commands[] = {
    {"NICK", "nick", &Irc::Nick},         {"USER", "user", &Irc::User},
    {"QUIT", "quit", &Irc::Quit},         {"CAP", "cap", &Irc::Cap},
    {"PING", "ping", &Irc::Ping},         {"PASS", "pass", &Irc::Pass},
    {"PRIVMSG", "privmsg", &Irc::Privmsg}};

Irc::Irc(const char *port, const char *password)
    : _socket_in_use(0),
      _connected_clients(0),
      _port(port),
      _password(password),
      _address() {
	const int opt = 1;
	// socket creation
	// AF_INET = ipv4
	// SOCK_STREAM = TCP and SOCK_NONBLOCK = I/O non-blocking
	// protocol 0 is to target ips
	_server_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_server_socket < 0)
		throw std::invalid_argument(strerror(errno));

	// https://stackoverflow.com/questions/21515946/what-is-sol-socket-used-for
	if (setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
	               &opt, sizeof(opt)))
		throw std::invalid_argument(strerror(errno));

	// Settings for incoming connection from client
	// Acceptation ipv4 AF_INET
	// From any address INADDR_ANY
	// Only on one defined port => htons(atoi(port))
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(std::strtol(_port.c_str(), NULL, 10));

	// apply the settings on our server socket (bind())
	if (bind(_server_socket, reinterpret_cast<sockaddr *>(&_address),
	         sizeof(_address)) < 0)
		throw std::invalid_argument(strerror(errno));

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
		throw std::invalid_argument(strerror(errno));

	std::cout << "[SERVER] Listening on port "
			<< _port
			<< " | fd=" << _server_socket
			<< std::endl;
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
					while (client.HasPendingCommand()) RunCommand(client);
				}
			} catch (ClientClose &reason) {
				std::cout << "[DISCONNECT] fd=" << _socket_in_use
						<< " | reason=" << reason.what()
						<< std::endl;

				close(_socket_in_use);
				_clients.erase(_clients.find(_socket_in_use));
				_pollfds.erase(_pollfds.begin() + i + 1);
				_connected_clients--;
			}
		}
	}
}

void Irc::ConnectClient() {
	_socket_in_use = accept(_server_socket,
	                        reinterpret_cast<sockaddr *>(&_address), &_addrlen);

	const pollfd pollfd = {
	    .fd = _socket_in_use,
	    .events = POLLIN,
	    .revents = 0,
	};

	_clients[_socket_in_use] = Client(_address);
	_pollfds.push_back(pollfd);
	_connected_clients++;

	std::cout << "[CONNECT] fd=" << _socket_in_use
          << " | clients=" << _connected_clients
          << std::endl;
}

bool Irc::HandleRecv() {
	uint8_t recv_buffer[MAX_LENGTH + 1];
	const ssize_t recv_size = recv(_socket_in_use, recv_buffer, MAX_LENGTH, 0);

	if (recv_size == 0)
		throw ClientClose("user asked for disconnect");
	if (recv_size < 0)
		throw std::runtime_error("failed recv for client");

	_clients[_socket_in_use].PushBuffer(recv_buffer, recv_size);

	std::cout << "[RECV] fd=" << _socket_in_use
			<< " | bytes=" << recv_size
			<< std::endl;
	return true;
}

void Irc::RunCommand(Client &client) {
	std::vector<std::string> args = client.CreateArgs();

	if (args.empty())
		return;

	bool has_hit = false;
	const std::string cmd = args[0];
	args.erase(args.begin());
	std::cout << "[CMD] fd=" << _socket_in_use
          << " | " << cmd
          << std::endl;
	const int max = sizeof(_commands) / sizeof(_commands[0]);

	for (int i = 0; i < max; ++i) {
		if (cmd == _commands[i].lower || cmd == _commands[i].upper) {
			(this->*_commands[i].handler)(args, client);
			has_hit = true;
			break;
		}
	}

	if (!has_hit) {
		std::string message = LOCALHOST + "421 ERR_UNKNOWNCOMMAND " + cmd +
		                      " :Unknown command" + CR_LF;
		send(_socket_in_use, message.c_str(), message.size(), 0);
		return;
	}

	if (client.NeedWelcome())
		return SendWelcome(client);
}

void Irc::SendWelcome(Client &client) const {
	const std::string message = LOCALHOST + "001 " + client.GetNickname() +
	                            " :Welcome to a random IRC Server" + CR_LF;
	send(_socket_in_use, message.c_str(), message.size(), 0);
}

Irc::~Irc() {}
