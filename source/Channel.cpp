#include "Channel.hpp"

#include <algorithm>

Channel::Channel(const std::string& name, int creator_fd)
    : _name(name), _topic_restricted(false), _invite_only(false) {
	_clients.push_back(creator_fd);
	_operators.push_back(creator_fd);
}

Channel::~Channel() {}

const std::string& Channel::GetName() const {
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

const std::vector<int>& Channel::GetClients() const {
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

const std::string& Channel::GetTopic() const {
	return _topic;
}

void Channel::SetTopic(const std::string& topic) {
	_topic = topic;
}

bool Channel::IsTopicRestricted() const {
	return _topic_restricted;
}

void Channel::SetTopicRestricted(bool restricted) {
	_topic_restricted = restricted;
}

bool Channel::IsInviteOnly() const {
	return _invite_only;
}

void Channel::SetInviteOnly(bool enabled) {
	_invite_only = enabled;
}

void Channel::AddInvitation(int fd) {
	if (!IsInvited(fd))
		_invited.push_back(fd);
}

bool Channel::IsInvited(int fd) const {
	return std::find(_invited.begin(), _invited.end(), fd) != _invited.end();
}

void Channel::RemoveInvitation(int fd) {
	std::vector<int>::iterator it =
	    std::find(_invited.begin(), _invited.end(), fd);
	if (it != _invited.end())
		_invited.erase(it);
}
