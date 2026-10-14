#include "Client.hpp"

// Constructors / Destructor

Client::Client()
    : _nickname("*"),
      _has_set_pwd(false),
      _is_registered(false),
      _close_state(OPEN) {}

Client::~Client() {}

// IRC buffer / parsing

void Client::PushBuffer(uint8_t *c, const size_t &size) {
	_buffer.append(reinterpret_cast<char *>(c), size);

	const size_t last_end = _buffer.rfind('\n');
	const size_t pending_size = last_end == string::npos
	                                ? _buffer.size()
	                                : _buffer.size() - (last_end + 1);

	if (pending_size > MAX_BUFFER_SIZE - 1)
		MarkClosing("Input too long");
}

bool Client::HasPendingCommand() const {
	return _buffer.find('\n') != string::npos;
}

vector<string> Client::CreateArgs() {
	const size_t line_end = _buffer.find('\n');

	if (line_end == string::npos)
		return vector<string>();

	if (line_end + 1 > MAX_BUFFER_SIZE) {
		MarkClosing("Input too long");
		return vector<string>();
	}

	string line = _buffer.substr(0, line_end);
	_buffer.erase(0, line_end + 1);

	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	return ParseLine(line);
}

vector<string> Client::ParseLine(const string &line) {
	vector<string> result;
	size_t pos = 0;

	while (pos < line.size() && line[pos] == ' ') ++pos;

	if (pos < line.size() && line[pos] == ':') {
		while (pos < line.size() && line[pos] != ' ') ++pos;
		while (pos < line.size() && line[pos] == ' ') ++pos;
	}

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

		while (pos < line.size() && line[pos] == ' ') ++pos;
	}

	return result;
}

void Client::QueueOutput(const string &message) {
	_output_buffer += message;
}

bool Client::HasPendingOutput() const {
	return !_output_buffer.empty();
}

const string &Client::GetOutputBuffer() const {
	return _output_buffer;
}

void Client::ConsumeOutput(size_t size) {
	_output_buffer.erase(0, size);
}

// Identity
string Client::Identity() const {
	return _nickname + '!' + _user;
}

const string &Client::GetNickname() const {
	return _nickname;
}

const string &Client::GetLower() const {
	return _lower_nickname;
}

void Client::SetNickname(const string &value) {
	_nickname = value.substr(0, 20);
	_lower_nickname = ToLower(_nickname);
}

string &Client::GetUser() {
	return _user;
}

void Client::SetUser(const vector<string> &values) {
	_user = ToValidUsername(values[0]);
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

bool Client::SetRegistered() {
	if (_nickname != "*" && !_nickname.empty() && _has_set_pwd &&
	    !_user.empty() && !_fullname.empty() && !_mode.empty())
		_is_registered = true;
	return _is_registered;
}

bool Client::IsRegistered() const {
	return _is_registered;
}

bool Client::IsClosing() const {
	return _close_state != OPEN;
}

void Client::MarkClosing(const string &reason) {
	if (_close_state != OPEN)
		return;
	_close_state = DRAIN;
	_quit_reason = reason;
}

void Client::MarkDead(const string &reason) {
	if (_close_state == OPEN)
		_quit_reason = reason;
	_close_state = NOW;
}

bool Client::ShouldClose() const {
	return _close_state == NOW ||
	       (_close_state == DRAIN && _output_buffer.empty());
}

const string &Client::QuitReason() const {
	return _quit_reason;
}