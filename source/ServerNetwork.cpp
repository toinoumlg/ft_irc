#include "Server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <new>

using namespace std;

const int MAX_LENGTH = 512;

// Network I/O
// [francais] Modification: conserve l'adresse recue par accept, borne les
// admissions et annule l'ajout en cas d'échec d'allocation sans conserver de
// socket oubliee.
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

	// [francais] Annule l'ajout dans les deux conteneurs si l'admission du
	// client ne peut pas être terminée.
	if (_connected_clients >= 1024) {
		close(_socket_in_use);
		return;
	}
	try {
		_clients[_socket_in_use].SetHost(inet_ntoa(client_address.sin_addr));
		_pollfds.push_back(client_pollfd);
		_connected_clients++;
	} catch (const bad_alloc&) {
		_clients.erase(_socket_in_use);
		close(_socket_in_use);
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

void Server::HandleRecv(Client& client) const {
	uint8_t recv_buffer[MAX_LENGTH + 1];
	const ssize_t recv_size = recv(_socket_in_use, recv_buffer, MAX_LENGTH, 0);

	if (recv_size == 0)
		return client.MarkClosing("Client disconnect");

	if (recv_size < 0)
		return client.MarkDead("Read error");

	client.PushBuffer(recv_buffer, recv_size);
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

// Disconnection / cleanup
// [francais] Modification: retire appartenances et invitations puis
// supprime les canaux vides par échange, sans copie allouante pendant la
// récupération mémoire.
void Server::RemoveClientFromChannels() {
	for (size_t i = 0; i < _channels.size();) {
		_channels[i].RemoveInvitation(_socket_in_use);
		if (_channels[i].HasClient(_socket_in_use))
			_channels[i].RemoveClient(_socket_in_use);

		if (_channels[i].IsEmpty()) {
			_channels[i].Swap(_channels.back());
			_channels.pop_back();
		} else
			++i;
	}
}

// [francais] Modification: poursuit le nettoyage de la connexion
// meme si la notification QUIT échoue faute de mémoire.
void Server::RemoveClient(int& i) {
	try {
		NotifyQuit();
	} catch (const bad_alloc&) {
	}
	RemoveClientFromChannels();
	close(_socket_in_use);
	_clients.erase(_clients.find(_socket_in_use));
	_pollfds.erase(_pollfds.begin() + i + 1);
	_connected_clients--;
	i--;
}
