#include "Client.hpp"

// Constructors / Destructor

Client::Client()
	: _client_addr(), _has_set_pwd(false), _is_registered(false) {}

Client::Client(const sockaddr_in &clientAddr)
	: _client_addr(clientAddr), _has_set_pwd(false), _is_registered(false) {}

Client::~Client() {}


// Connection

const char *Client::Address() const {
	char test[1024];

	return inet_ntop(AF_INET, &_client_addr.sin_addr.s_addr, test, 1024);
}

in_port_t Client::Port() const {
	return _client_addr.sin_port;
}


// IRC buffer / parsing

void Client::PushBuffer(uint8_t *c, const size_t &size) {
	_buffer.append(reinterpret_cast<char *>(c), size);

	const size_t last_end = _buffer.rfind("\r\n");
	size_t pending_size;

	if (last_end == string::npos)
		pending_size = _buffer.size();
	else
		pending_size = _buffer.size() - (last_end + 2);

	if (pending_size > MAX_BUFFER_SIZE - 2)
		throw Close();
}

bool Client::HasPendingCommand() const {
	return _buffer.find("\r\n") != string::npos;
}

vector<string> Client::CreateArgs() {
	const size_t line_end = _buffer.find("\r\n");

	if (line_end == string::npos)
		return vector<string>();

	if (line_end + 2 > MAX_BUFFER_SIZE)
		throw Close();

	const string line = _buffer.substr(0, line_end);
	_buffer.erase(0, line_end + 2);

	return ParseLine(line);
}

vector<string> Client::ParseLine(const string &line) const {
	vector<string> result;
	size_t pos = 0;

	while (pos < line.size() && line[pos] == ' ')
		++pos;

	while (pos < line.size()) {
		if (line[pos] == ':') {
			result.push_back(line.substr(pos + 1));
			break;
		}

		const size_t end = line.find(' ', pos);

		if (end == string::npos) {
			result.push_back(line.substr(pos));
			break;
		}

		result.push_back(line.substr(pos, end - pos));
		pos = end + 1;

		while (pos < line.size() && line[pos] == ' ')
			++pos;
	}

	return result;
}


// Identity

string Client::Identity() const {
	return ":" + _nickname + "!~" + _user;
}

string &Client::GetNickname() {
	return _nickname;
}

void Client::SetNickname(const string &value) {
	_nickname = value;
}

string &Client::GetUser() {
	return _user;
}

void Client::SetUser(vector<string> &values) {
	_user = values[0];
	_mode = values[1];
	_fullname = values[3];
}

string &Client::GetAlias() {
	return _alias;
}

void Client::SetAlias(const string &value) {
	_alias = value;
}


// Registration

void Client::SetPassword(const bool value) {
	_has_set_pwd = value;
}

bool Client::HasSetPassword() const {
	return _has_set_pwd;
}

void Client::SetRegistered(bool value) {
	_is_registered = value;
}

bool Client::IsRegistered() const {
	return _is_registered;
}


// Stream operator

ostream &operator<<(ostream &stream, const Client &client) {
	stream << client.Address() << ":" << client.Port();
	return stream;
}