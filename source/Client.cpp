#include "Client.hpp"

#include "Irc.hpp"

Client::Client() : _clientAddr(), _is_initialized(false) {}

Client::Client(const sockaddr_in &clientAddr)
    : _clientAddr(clientAddr), _is_initialized(false) {}

std::vector<std::string> Client::CreateArgs() {
	std::vector<std::string> args = Split();

	_buffer = _buffer.substr(_buffer.find('\n') + 1, _buffer.size());
	return args;
};

const char *Client::Address() const {
	char test[1024];

	return inet_ntop(AF_INET, &_clientAddr.sin_addr.s_addr, test, 1024);
}

in_port_t Client::Port() const {
	return _clientAddr.sin_port;
}

void Client::PushBuffer(uint8_t *c, const size_t &size) {
	_buffer.append(reinterpret_cast<char *>(c), size);
	if (_buffer.length() >= MAX_BUFFER_SIZE)
		throw ClientClose("command exceed max buffer size");
}

bool Client::HasPendingCommand() const {
	const size_t cr = _buffer.find('\r');
	const size_t lf = _buffer.find('\n');

	if (cr && lf == cr + 1)
		return true;
	return false;
}

bool Client::IsValid() const {
	return _nickname.empty() || _nickname.length() > 9 || _alias.empty() ||
	       _alias.length() > 10;
}

void Client::SetPassword(const bool value) {
	_has_set_pwd = value;
}
bool Client::HasSetPassword() const {
	return _has_set_pwd;
}

bool Client::NeedWelcome() {
	if (!_is_initialized && !_nickname.empty() && !_user.empty() &&
	    _has_set_pwd) {
		_is_initialized = true;
		return true;
	}
	return false;
}
bool Client::IsRegistered() const {
	return _is_initialized;
}

std::string &Client::GetNickname() {
	return _nickname;
}

void Client::SetNickname(std::string &value) {
	_nickname = value;
}

std::string &Client::GetUser() {
	return _user;
}

void Client::SetUser(std::string &value) {
	_user = value;
}

std::string &Client::GetAlias() {
	return _alias;
}

void Client::SetAlias(std::string &value) {
	_alias = value;
}

Client::~Client() {}

// https://www.geeksforgeeks.org/cpp/how-to-split-string-by-delimiter-in-cpp/
std::vector<std::string> Client::Split() const {
	std::stringstream ss(_buffer);
	std::vector<std::string> result;
	std::string token;

	while (getline(ss, token, ' ')) {
		const size_t end = token.find('\r');
		if (end != std::string::npos)
			token = token.substr(0, end);
		result.push_back(token);
	}

	return result;
}

std::ostream &operator<<(std::ostream &stream, const Client &client) {
	std::cout << client.Address() << ":" << client.Port();
	return stream;
}
