#include "Channel.hpp"

#include <algorithm>

Channel::Channel(const std::string &name, int creator_fd)
	: _name(name) {
	_clients.push_back(creator_fd);
	_operators.push_back(creator_fd);
}

Channel::~Channel() {}

const std::string &Channel::GetName() const {
	return _name;
}

bool Channel::HasClient(int fd) const {
	return std::find(_clients.begin(), _clients.end(), fd) != _clients.end();
}

void Channel::AddClient(int fd) {
	if (!HasClient(fd))
		_clients.push_back(fd);
}

void Channel::RemoveClient(int fd) {
	std::vector<int>::iterator it =
		std::find(_clients.begin(), _clients.end(), fd);

	if (it != _clients.end())
		_clients.erase(it);

	RemoveOperator(fd);
}

const std::vector<int> &Channel::GetClients() const {
	return _clients;
}

bool Channel::IsOperator(int fd) const {
	return std::find(_operators.begin(), _operators.end(), fd) !=
	       _operators.end();
}

void Channel::AddOperator(int fd) {
	if (HasClient(fd) && !IsOperator(fd))
		_operators.push_back(fd);
}

void Channel::RemoveOperator(int fd) {
	std::vector<int>::iterator it =
		std::find(_operators.begin(), _operators.end(), fd);

	if (it != _operators.end())
		_operators.erase(it);
}

bool Channel::IsEmpty() const {
	return _clients.empty();
}