#pragma once

#include <string>
#include <netinet/in.h>
#include <bits/stdc++.h>
#include <vector>
#include <arpa/inet.h>

const int MAX_BUFFER_SIZE = 512;


class Client {
public:
    Client();

    Client(const sockaddr_in &clientAddr);

    ~Client();

    const char *Address() const;

    in_port_t Port() const;

    void PushBuffer(uint8_t *c, const ssize_t &size);

    bool HasPendingCommand() const;

    bool IsValid() const;

    void SetNickname(std::string &value);

    std::vector<std::string> CreateArgs();

private:
    std::vector<std::string> Split() const;

    std::string _buffer;
    sockaddr_in _clientAddr;
    std::string _nickname;
    std::string _alias;
};

std::ostream &operator<<(std::ostream &stream, const Client &client);
