#include "Client.hpp"

#include "Irc.hpp"

Client::Client() : _clientAddr() {
}

Client::Client(const sockaddr_in &clientAddr) : _clientAddr(clientAddr) {
}

std::vector<std::string> Client::CreateArgs() {
    std::vector<std::string> args = Split();

    _buffer = _buffer.substr(_buffer.find('\n') + 1,
                             _buffer.size());
    return args;
};


const char *Client::Address() const {
    char test[1024];

    return inet_ntop(AF_INET, &_clientAddr.sin_addr.s_addr, test, 1024);
}

in_port_t Client::Port() const {
    return _clientAddr.sin_port;
}

void Client::PushBuffer(uint8_t *c, const ssize_t &size) {
    _buffer.append(reinterpret_cast<char *>(c), size);
    if (_buffer.length() >= MAX_BUFFER_SIZE)
        throw ClientClose("command exceed max buffer size");
}


bool Client::HasPendingCommand() const {
    const size_t br = _buffer.find('\r');
    const size_t cr = _buffer.find('\n');

    if (br && cr == br + 1)
        return true;
    return false;
}


bool Client::IsValid() const {
    return _nickname.empty() || _nickname.length() > 9 || _alias.empty() ||
           _alias.
           length() > 10;
}

void Client::SetNickname(std::string &value) {
    _nickname = value;
}

Client::~Client() {
}

// https://www.geeksforgeeks.org/cpp/how-to-split-string-by-delimiter-in-cpp/
std::vector<std::string> Client::Split() const {
    std::stringstream ss(_buffer);
    std::vector<std::string> result;
    std::string token;

    while (getline(ss, token, ' ')) {
        size_t end = token.find('\r');
        if (end > 0)
            token = token.substr(0, end);
        result.push_back(token);
    }

    return result;
}

std::ostream &operator<<(std::ostream &stream, const Client &client) {
    std::cout << client.Address() << ":" << client.Port();
    return stream;
}
