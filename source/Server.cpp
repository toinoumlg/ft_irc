#include "Server.hpp"

static volatile sig_atomic_t gSignalStatus;

const Server::CommandEntry Server::_commands[] = {
    {"NICK", false, &Server::Nick},      {"USER", false, &Server::User},
    {"PASS", false, &Server::Pass},      {"CAP", false, &Server::Cap},
    {"QUIT", false, &Server::Quit},      {"PING", true, &Server::Ping},
    {"PRIVMSG", true, &Server::Privmsg}, {"JOIN", true, &Server::Join},
    {"KICK", true, &Server::Kick},       {"TOPIC", true, &Server::Topic},
    {"INVITE", true, &Server::Invite},   {"MODE", true, &Server::Mode},
    {"PART", true, &Server::Part}};

bool Server::CheckPort(const char* port) {
	if (port == NULL || !isdigit(static_cast<unsigned char>(port[0])))
		return false;
	char* end = NULL;
	errno = 0;
	const long int value = strtol(port, &end, 10);
	if (*end != '\0' || value < 1024 || value > 65535)
		return false;
	return true;
}

static void signal_handler(int signal) {
	gSignalStatus = signal;
}

static void SetSignals() {
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGPIPE, SIG_IGN);
}

static void CloseAndThrow(const int fd) {
	const string error = strerror(errno);
	close(fd);
	throw invalid_argument(error);
}

Server::Server(const string& port, const string& password)
    : _socket_in_use(0),
      _connected_clients(0),
      _port(port),
      _password(password) {
	SetSignals();
	const int opt = 1;
	// socket creation
	// AF_INET = ipv4
	// SOCK_STREAM = TCP and SOCK_NONBLOCK = I/O non-blocking
	// protocol 0 is to target ips
	_server_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_server_socket < 0)
		throw invalid_argument(strerror(errno));

	// https://stackoverflow.com/questions/21515946/what-is-sol-socket-used-for
	if (setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &opt,
	               sizeof(opt)) < 0)
		CloseAndThrow(_server_socket);

	sockaddr_in address = {};
	// Settings for incoming connection from client
	// Acceptation ipv4 AF_INET
	// From any address INADDR_ANY
	// Only on one defined port => htons(atoi(port))
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(strtol(_port.c_str(), NULL, 10));

	// apply the settings on our server socket (bind())
	if (bind(_server_socket, reinterpret_cast<sockaddr*>(&address),
	         sizeof(address)) < 0)
		CloseAndThrow(_server_socket);

	// create the server pollfd description
	pollfd server;

	server.fd = _server_socket;
	server.events = POLLIN;
	server.revents = 0;

	_pollfds.push_back(server);

	// enable passive listening of our server with 3 inside the queue
	if (listen(_server_socket, 3) < 0)
		CloseAndThrow(_server_socket);
}

const int MAX_LENGTH = 512;

void Server::Run() {
	while (gSignalStatus == 0) {
		const int res = poll(&_pollfds[0], _pollfds.size(), 300);

		if (res < 0)
			throw runtime_error(strerror(errno));

		if (res == 0)
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

void Server::ProcessClient(const short client_event, int& i) {
	Client& client = _clients[_socket_in_use];

	if (client_event & (POLLERR | POLLHUP | POLLNVAL))
		client.MarkDead("Connection error");
	else {
		if (client_event & POLLIN) {
			HandleRecv(client);
			while (!client.IsClosing() && client.HasPendingCommand())
				RunCommand(client);
		}

		if (client_event & POLLOUT)
			HandleSend(client);
	}
	if (client.IsClosing())
		DisablePollin(_socket_in_use);
	if (client.ShouldClose())
		RemoveClient(i);
}

void Server::RemoveClient(int& i) {
	NotifyQuit();
	RemoveClientFromChannels();
	close(_socket_in_use);
	_clients.erase(_clients.find(_socket_in_use));
	_pollfds.erase(_pollfds.begin() + i + 1);
	_connected_clients--;
	i--;
}

void Server::ConnectClient() {
	sockaddr_in client_address;
	socklen_t client_addrlen = sizeof(client_address);

	_socket_in_use =
	    accept(_server_socket, reinterpret_cast<sockaddr*>(&client_address),
	           &client_addrlen);

	if (_socket_in_use < 0)
		return;

	if (fcntl(_socket_in_use, F_SETFL, O_NONBLOCK) < 0) {
		close(_socket_in_use);
		return;
	}

	pollfd client_pollfd;
	client_pollfd.fd = _socket_in_use;
	client_pollfd.events = POLLIN;
	client_pollfd.revents = 0;

	_clients[_socket_in_use];
	_pollfds.push_back(client_pollfd);
	_connected_clients++;
}

void Server::HandleRecv(Client& client) const {
	uint8_t recv_buffer[MAX_LENGTH + 1];
	const ssize_t recv_size = recv(_socket_in_use, recv_buffer, MAX_LENGTH, 0);

	if (recv_size == 0)
		return client.MarkClosing("Client disconnect");

	if (recv_size < 0)
		return client.MarkDead("Read error");

	client.PushBuffer(recv_buffer, recv_size);
}

void Server::RunCommand(Client& client) {
	vector<string> args = client.CreateArgs();

	if (args.empty() || args[0].empty())
		return;

	string cmd;
	for (string::const_iterator it = args[0].begin(); it != args[0].end(); ++it)
		cmd += static_cast<char>(toupper(*it));

	args.erase(args.begin());
	const int max = sizeof(_commands) / sizeof(_commands[0]);
	for (int i = 0; i < max; ++i)
		if (cmd == _commands[i].name) {
			if (_commands[i].needsRegistration && !client.IsRegistered())
				return QueueMessage(Response::Build(
				    ERR_NOTREGISTERED, client, ":You have not registered"));

			return (this->*_commands[i].handler)(args, client);
		}

	QueueMessage(
	    Response::Build(ERR_UNKNOWNCOMMAND, client, cmd + " :Unknown command"));
}

void Server::HandleSend(Client& client) {
	if (!client.HasPendingOutput())
		return;

	const string& buffer = client.GetOutputBuffer();

	const ssize_t sent = send(_socket_in_use, buffer.c_str(), buffer.size(), 0);

	if (sent < 0)
		return client.MarkDead("Write error");

	client.ConsumeOutput(static_cast<size_t>(sent));

	if (!client.HasPendingOutput())
		DisablePollout(_socket_in_use);
}

void Server::EnablePollout(int fd) {
	for (size_t i = 1; i < _pollfds.size(); ++i) {
		if (_pollfds[i].fd == fd) {
			_pollfds[i].events |= POLLOUT;
			return;
		}
	}
}

void Server::DisablePollout(int fd) {
	for (size_t i = 1; i < _pollfds.size(); ++i) {
		if (_pollfds[i].fd == fd) {
			_pollfds[i].events &= ~POLLOUT;
			return;
		}
	}
}

void Server::DisablePollin(const int fd) {
	for (size_t i = 1; i < _pollfds.size(); ++i) {
		if (_pollfds[i].fd == fd) {
			_pollfds[i].events &= ~POLLIN;
			return;
		}
	}
}

void Server::RemoveClientFromChannels() {
	for (size_t i = 0; i < _channels.size();) {
		_channels[i].RemoveInvitation(_socket_in_use);
		if (_channels[i].HasClient(_socket_in_use))
			_channels[i].RemoveClient(_socket_in_use);

		if (_channels[i].IsEmpty())
			_channels.erase(_channels.begin() + i);
		else
			++i;
	}
}

void Server::NotifyWelcome() {
	QueueMessage(Response::Build(RPL_WELCOME, _clients[_socket_in_use],
	                             ":Welcome to a random IRC server !"));
}

void Server::NotifyQuit() {
	const Client& client = _clients[_socket_in_use];
	if (!client.IsRegistered())
		return;

	const string message =
	    Response::Build(client, "QUIT", ':' + client.QuitReason());

	vector<int> notified;
	notified.push_back(_socket_in_use);

	for (size_t i = 0; i < _channels.size(); ++i)
		if (_channels[i].HasClient(_socket_in_use))
			BroadcastMessages(_channels[i].GetClients(), message, notified);
}

vector<Channel>::iterator Server::GetChannelByName(const string& name) {
	const string lower = ToLower(name);
	for (vector<Channel>::iterator it = _channels.begin();
	     it != _channels.end(); ++it)
		if (it->GetLower() == lower)
			return it;
	return _channels.end();
}

int Server::GetClientByNick(const string& nickname) {
	const string lower = ToLower(nickname);
	for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end();
	     ++it) {
		if (it->second.IsRegistered() && it->second.GetLower() == lower)
			return it->first;
	}
	return -1;
}

vector<string> Server::SplitTargets(string& args) {
	vector<string> targets;
	while (args.begin() != args.end()) {
		const size_t pos = args.find(',');
		if (pos == string::npos) {
			targets.push_back(args);
			break;
		}
		targets.push_back(args.substr(0, pos));
		args.erase(0, pos + 1);
	}
	return targets;
}

vector<pair<string, string> > Server::SplitChannelsAndKeys(
    vector<string>& args) {
	vector<pair<string, string> > channels;

	string& names = args[0];
	while (names.begin() != names.end()) {
		const size_t pos = names.find(',');
		if (pos == string::npos) {
			channels.push_back(pair<string, string>(names, ""));
			break;
		}
		channels.push_back(pair<string, string>(names.substr(0, pos), ""));
		names.erase(0, pos + 1);
	}
	if (args.size() == 1)
		return channels;

	string& keys = args[1];
	vector<pair<string, string> >::iterator it = channels.begin();
	while (keys.begin() != keys.end() && it != channels.end()) {
		const size_t pos = keys.find(',');
		if (pos == string::npos) {
			it->second = keys;
			break;
		}
		it->second = keys.substr(0, pos);
		keys.erase(0, pos + 1);
		++it;
	}

	return channels;
}

Server::~Server() {
	for (vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end();
	     ++it)
		close(it->fd);
}
