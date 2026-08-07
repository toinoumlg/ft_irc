#include "Channel.hpp"

Channel::Channel(const std::string& name, int creatorfd) : _name(name) {
	_clients.push_back(creatorfd);
}

Channel::~Channel() {}