#include "Server.hpp"

const Server::CommandEntry Server::_commands[] = {
    {"NICK", "nick", &Server::Nick},         {"USER", "user", &Server::User},
    {"QUIT", "quit", &Server::Quit},         {"CAP", "cap", &Server::Cap},
    {"PING", "ping", &Server::Ping},         {"PASS", "pass", &Server::Pass},
    {"PRIVMSG", "privmsg", &Server::Privmsg}};

Server::Server(const char *port, const char *password)
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
		throw invalid_argument(strerror(errno));

	// https://stackoverflow.com/questions/21515946/what-is-sol-socket-used-for
	if (setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
	               &opt, sizeof(opt)))
		throw invalid_argument(strerror(errno));

	// Settings for incoming connection from client
	// Acceptation ipv4 AF_INET
	// From any address INADDR_ANY
	// Only on one defined port => htons(atoi(port))
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(strtol(_port.c_str(), NULL, 10));

	// apply the settings on our server socket (bind())
	if (bind(_server_socket, reinterpret_cast<sockaddr *>(&_address),
	         sizeof(_address)) < 0)
		throw invalid_argument(strerror(errno));

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
		throw invalid_argument(strerror(errno));
}

const int MAX_LENGTH = 512;

void Server::Run() {
	while (true) {
		const int res = poll(&_pollfds[0], _pollfds.size(), 300);

		if (!res)
			continue;

		if (_pollfds[0].revents & POLLIN)
			ConnectClient();

		for (int i = 0; i < _connected_clients; ++i) {
			const short client_event = _pollfds[i + 1].revents;
			_socket_in_use = _pollfds[i + 1].fd;
			ProcessClient(client_event, i);
		}
	}
}

void Server::ProcessClient(const short client_event, int &i) {
	try {
		if (client_event & (POLLERR | POLLHUP | POLLNVAL))
			throw Client::Close();

		if (client_event & POLLIN) {
			HandleRecv();

			Client &client = _clients[_socket_in_use];
			while (client.HasPendingCommand()) RunCommand(client);
		}
	} catch (const Client::Close &c) {
		cout << "Client " << _clients[_socket_in_use] << c.what() << endl;
		close(_socket_in_use);
		_clients.erase(_clients.find(_socket_in_use));
		_pollfds.erase(_pollfds.begin() + i + 1);
		_connected_clients--;
		i--;
	}
}

void Server::ConnectClient() {
	_socket_in_use = accept(_server_socket,
	                        reinterpret_cast<sockaddr *>(&_address), &_addrlen);
	if (_socket_in_use < 0)
		throw std::runtime_error(strerror(errno));

	const pollfd pollfd = {
	    .fd = _socket_in_use,
	    .events = POLLIN,
	    .revents = 0,
	};

	_clients[_socket_in_use] = Client(_address);
	_pollfds.push_back(pollfd);
	_connected_clients++;
}

void Server::HandleRecv() {
	uint8_t recv_buffer[MAX_LENGTH + 1];
	const ssize_t recv_size = recv(_socket_in_use, recv_buffer, MAX_LENGTH, 0);

	if (recv_size == 0)
		throw Client::Close();

	if (recv_size < 0)
		throw runtime_error("failed recv for client");

	_clients[_socket_in_use].PushBuffer(recv_buffer, recv_size);
}

void Server::RunCommand(Client &client) {
	vector<string> args = client.CreateArgs();

	if (args.empty())
		return;

	bool has_hit = false;
	const string cmd = args[0];
	args.erase(args.begin());
	const int max = sizeof(_commands) / sizeof(_commands[0]);

	for (int i = 0; i < max; ++i)
		if (cmd == _commands[i].lower || cmd == _commands[i].upper) {
			(this->*_commands[i].handler)(args, client);
			has_hit = true;
			break;
		}

	if (!has_hit)
		return Response::Send(_socket_in_use, StatusCode::ERR_UNKNOWNCOMMAND,
		                      cmd + " :Unknown command");
}

void Server::Privmsg(vector<string> &args, Client &client) {
	if (args.size() < 2) {
		return;
	}

	string &target = args[0];
	for (ClientMap::iterator it = _clients.begin(); it != _clients.end();
	     ++it) {
		if (target == it->second.GetNickname()) {
			string message;

			for (size_t i = 1; i < args.size() - 1; ++i)
				message += args[i] + " ";
			message += args.back();
			Response::Send(it->first, "PRIVMSG " + client.GetNickname(),
			               message);
			return;
		}
	}

	Response::Send(_socket_in_use, StatusCode::ERR_NOSUCHNICK,
	               client.GetNickname() + " :No such nick/channel");
}

Server::~Server() {}
