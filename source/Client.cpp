#include "Client.hpp"

Client::Client() : _client_addr(), _has_set_pwd(false), _is_registered(false) {}

Client::Client(const sockaddr_in &clientAddr)
    : _client_addr(clientAddr), _has_set_pwd(false), _is_registered(false) {}

vector<string> Client::CreateArgs() {
	vector<string> args = Split();

	_buffer = _buffer.substr(_buffer.find('\n') + 1, _buffer.size());

	return args;
};

const char *Client::Address() const {
	char test[1024];

	return inet_ntop(AF_INET, &_client_addr.sin_addr.s_addr, test, 1024);
}

in_port_t Client::Port() const {
	return _client_addr.sin_port;
}

void Client::PushBuffer(uint8_t *c, const size_t &size) {
	_buffer.append(reinterpret_cast<char *>(c), size);
	if (_buffer.length() >= MAX_BUFFER_SIZE)
		throw Close();
}
string Client::Identity() const {
	string identity = ":" + _nickname + "!~" + _user;
	return identity;
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
void Client::SetRegistered(bool value) {
	_is_registered = value;
}

bool Client::IsRegistered() const {
	return _is_registered;
}

string &Client::GetNickname() {
	return _nickname;
}

void Client::SetNickname(const string &value) {
	// should silently truncate if value > max server nickname length
	_nickname = value;
}

string &Client::GetUser() {
	return _user;
}

void Client::SetUser(vector<string> &values) {
	_user = values[0];
	_mode = values[1];
	if (values[3][0] == ':')
		values[3] = values[3].substr(1, values[3].size());
	for (size_t i = 3; i < values.size() - 1; ++i) _fullname += values[i] + " ";

	_fullname += values.back();
}

string &Client::GetAlias() {
	return _alias;
}

void Client::SetAlias(const string &value) {
	_alias = value;
}

Client::~Client() {}

// https://www.geeksforgeeks.org/cpp/how-to-split-string-by-delimiter-in-cpp/
vector<string> Client::Split() const {
	stringstream ss(_buffer);
	vector<string> result;
	string token;

	while (getline(ss, token, ' ')) {
		const size_t end = token.find('\r');

		if (end != string::npos) {
			token = token.substr(0, end);
			result.push_back(token);
			break;
		}
		result.push_back(token);
	}

	for (size_t i = 0; i < result.size(); ++i) {
		cout << "[" << result[i] << "] ";
	}
	cout << endl;
	return result;
}

ostream &operator<<(ostream &stream, const Client &client) {
	cout << client.Address() << ":" << client.Port();
	return stream;
}
