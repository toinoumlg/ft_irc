#include "Channel.hpp"

#include <sstream>

Channel::Channel(const std::string& name, const int creator_fd)
    : _name(name),
      _topic_restricted(false),
      _invite_only(false),
      _max_client(0) {
	_clients.push_back(creator_fd);
	_operators.push_back(creator_fd);
	SetModes();
	_lower_name = ToLower(_name);
}

Channel::Channel(const std::string& name, const int creator_fd,
                 const std::string& key)
    : _name(name),
      _key(key),
      _topic_restricted(false),
      _invite_only(false),
      _max_client(0) {
	_clients.push_back(creator_fd);
	_operators.push_back(creator_fd);
	SetModes();
	_lower_name = ToLower(_name);
}

Channel::~Channel() {}

bool Channel::IsValidName(const std::string& name) {
	return !name.empty() &&
	       (name[0] == '#' || name[0] == '+' || name[0] == '!' ||
	        name[0] == '&') &&
	       name.size() < 50;
}

const std::string& Channel::GetName() const {
	return _name;
}

const std::string& Channel::GetLower() const {
	return _lower_name;
}

bool Channel::HasClient(const int fd) const {
	return std::find(_clients.begin(), _clients.end(), fd) != _clients.end();
}

void Channel::AddClient(const int fd) {
	if (!HasClient(fd))
		_clients.push_back(fd);
}

void Channel::RemoveClient(const int fd) {
	std::vector<int>::iterator it =
	    std::find(_clients.begin(), _clients.end(), fd);

	if (it != _clients.end())
		_clients.erase(it);

	RemoveOperator(fd);
}

const std::vector<int>& Channel::GetClients() const {
	return _clients;
}

bool Channel::IsOperator(const int fd) const {
	return std::find(_operators.begin(), _operators.end(), fd) !=
	       _operators.end();
}

void Channel::AddOperator(const int fd) {
	if (HasClient(fd) && !IsOperator(fd))
		_operators.push_back(fd);
}

void Channel::RemoveOperator(const int fd) {
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

void Channel::SetTopicRestricted(const bool restricted) {
	_topic_restricted = restricted;
}

bool Channel::IsInviteOnly() const {
	return _invite_only;
}

void Channel::SetInviteOnly(const bool enabled) {
	_invite_only = enabled;
}

void Channel::AddInvitation(const int fd) {
	if (!IsInvited(fd))
		_invited.push_back(fd);
}

bool Channel::IsInvited(const int fd) const {
	return std::find(_invited.begin(), _invited.end(), fd) != _invited.end();
}

void Channel::RemoveInvitation(const int fd) {
	const std::vector<int>::iterator it =
	    std::find(_invited.begin(), _invited.end(), fd);
	if (it != _invited.end())
		_invited.erase(it);
}

bool Channel::HasKey() const {
	return !_key.empty();
}

void Channel::SetKey(const std::string& key) {
	_key = key;
}

bool Channel::CheckKey(const std::string& key) const {
	return key == _key;
}

void Channel::SetMaxClient(const unsigned short max) {
	_max_client = max;
}

bool Channel::IsFull() const {
	return _max_client == 0 ? false : _clients.size() >= _max_client;
}
unsigned int Channel::Limit() const {
	return _max_client;
}

void Channel::SetModes() {
	std::string modes = "+";
	std::string params;
	if (IsInviteOnly())
		modes += 'i';
	if (IsTopicRestricted())
		modes += 't';
	if (HasKey()) {
		modes += 'k';
		params += " *";
	}
	if (Limit()) {
		ostringstream stream;
		stream << _max_client;
		modes += 'l';
		params += ' ' + stream.str();
	}
	_modes = modes + params;
}

std::string Channel::GetModes() const {
	return _modes;
}