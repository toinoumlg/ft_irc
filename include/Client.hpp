#pragma once

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>

#include <string>
#include <vector>

const int MAX_BUFFER_SIZE = 512;

class ClientClose : public std::exception {
   public:
	ClientClose(const char *msg) : _message(msg) {}

	const char *what() const throw() {
		return _message.c_str();
	}

	~ClientClose() throw() {};

   private:
	std::string _message;
};

class Client {
   public:
	Client();

	Client(const sockaddr_in &clientAddr);

	~Client();

	const char *Address() const;

	in_port_t Port() const;

	void PushBuffer(uint8_t *c, const size_t &size);

	bool HasPendingCommand() const;

	bool IsValid() const;

	void SetPassword(bool value);

	bool HasSetPassword() const;

	bool NeedWelcome();

	bool IsRegistered() const;

	std::string &GetNickname();

	void SetNickname(std::string &value);

	std::string &GetUser();

	void SetUser(std::string &value);

	std::string &GetAlias();

	void SetAlias(std::string &value);

	std::vector<std::string> CreateArgs();

   private:
	std::vector<std::string> Split() const;

	std::string _buffer;
	sockaddr_in _clientAddr;
	std::string _user;
	std::string _nickname;
	std::string _alias;
	bool _has_set_pwd;
	bool _is_initialized;
};

std::ostream &operator<<(std::ostream &stream, const Client &client);
