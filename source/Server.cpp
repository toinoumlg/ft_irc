#include "Server.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <new>
#include <stdexcept>

using namespace std;

static volatile sig_atomic_t gSignalStatus;

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

// [francais] Modification: mémorise la date de démarrage pour completer la
// réponse de bienvenue RPL_CREATED.
Server::Server(const string& port, const string& password)
    : _socket_in_use(0),
      _connected_clients(0),
      _port(port),
      _password(password) {
	SetSignals();
	const time_t now = time(NULL);
	char created[64];
	const tm* date = gmtime(&now);
	if (date &&
	    strftime(created, sizeof(created), "%Y-%m-%d %H:%M:%S UTC", date))
		_created = created;
	else
		_created = "unknown";
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

Server::~Server() {
	for (vector<pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end();
	     ++it)
		close(it->fd);
}

// Startup
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

// [francais] Modification: isole les echecs d'allocation par client,
// et distingue un arret demandé d'une erreur de poll.
void Server::Run() {
	while (gSignalStatus == 0) {
		const int res = poll(&_pollfds[0], _pollfds.size(), 300);
		if (res < 0) {
			if (gSignalStatus != 0)
				break;
			if (errno == EINTR)
				continue;
			throw runtime_error(strerror(errno));
		}
		if (_pollfds[0].revents & POLLIN)
			ConnectClient();
		for (int i = 0; i < _connected_clients; ++i) {
			_socket_in_use = _pollfds[i + 1].fd;
			try {
				ProcessClient(_pollfds[i + 1].revents, i);
			} catch (const bad_alloc&) {
				_clients.find(_socket_in_use)->second.Abort();
				RemoveClient(i);
			}
		}
	}
}
