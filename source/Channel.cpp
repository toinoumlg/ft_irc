#include "Channel.hpp"

#include <algorithm>
#include <sstream>

#include "Helper.hpp"

using namespace std;

Channel::Channel(const string& name, const int creator_fd)
    : _name(name),
      _max_client(0),
      _topic_restricted(false),
      _invite_only(false) {
	_clients.push_back(creator_fd);
	_operators.push_back(creator_fd);
	SetModes();
	_lower_name = ToLower(_name);
}

Channel::Channel(const string& name, const int creator_fd, const string& key)
    : _name(name),
      _max_client(0),
      _topic_restricted(false),
      _invite_only(false),
      _key(key) {
	_clients.push_back(creator_fd);
	_operators.push_back(creator_fd);
	SetModes();
	_lower_name = ToLower(_name);
}

Channel::~Channel() {}

// Name
bool Channel::IsValidName(const string& name) {
	return !name.empty() &&
	       (name[0] == '#' || name[0] == '+' || name[0] == '!' ||
	        name[0] == '&') &&
	       name.size() < 50;
}

const string& Channel::GetName() const {
	return _name;
}

const string& Channel::GetLower() const {
	return _lower_name;
}

// Members / capacity
bool Channel::HasClient(const int fd) const {
	return find(_clients.begin(), _clients.end(), fd) != _clients.end();
}

bool Channel::IsEmpty() const {
	return _clients.empty();
}

bool Channel::IsFull() const {
	return _max_client == 0 ? false : _clients.size() >= _max_client;
}

void Channel::AddClient(const int fd) {
	if (!HasClient(fd))
		_clients.push_back(fd);
}

void Channel::RemoveClient(const int fd) {
	vector<int>::iterator it = find(_clients.begin(), _clients.end(), fd);

	if (it != _clients.end())
		_clients.erase(it);

	RemoveOperator(fd);
}

void Channel::SetMaxClient(const unsigned short max) {
	_max_client = max;
}

const vector<int>& Channel::GetClients() const {
	return _clients;
}

unsigned int Channel::Limit() const {
	return _max_client;
}

// Operators
bool Channel::IsOperator(const int fd) const {
	return find(_operators.begin(), _operators.end(), fd) != _operators.end();
}

void Channel::AddOperator(const int fd) {
	if (HasClient(fd) && !IsOperator(fd))
		_operators.push_back(fd);
}

void Channel::RemoveOperator(const int fd) {
	vector<int>::iterator it = find(_operators.begin(), _operators.end(), fd);

	if (it != _operators.end())
		_operators.erase(it);
}

// Topic
bool Channel::IsTopicRestricted() const {
	return _topic_restricted;
}

void Channel::SetTopic(const string& topic) {
	_topic = topic;
}

void Channel::SetTopicRestricted(const bool restricted) {
	_topic_restricted = restricted;
}

const string& Channel::GetTopic() const {
	return _topic;
}

// Invitations
bool Channel::IsInviteOnly() const {
	return _invite_only;
}

bool Channel::IsInvited(const int fd) const {
	return find(_invited.begin(), _invited.end(), fd) != _invited.end();
}

void Channel::SetInviteOnly(const bool enabled) {
	_invite_only = enabled;
}

void Channel::AddInvitation(const int fd) {
	if (!IsInvited(fd))
		_invited.push_back(fd);
}

void Channel::RemoveInvitation(const int fd) {
	const vector<int>::iterator it = find(_invited.begin(), _invited.end(), fd);
	if (it != _invited.end())
		_invited.erase(it);
}

// Channel key
bool Channel::HasKey() const {
	return !_key.empty();
}

bool Channel::CheckKey(const string& key) const {
	return key == _key;
}

void Channel::SetKey(const string& key) {
	_key = key;
}

// Mode summary
void Channel::SetModes() {
	string modes = "+";
	string params;
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

string Channel::GetModes() const {
	return _modes;
}
