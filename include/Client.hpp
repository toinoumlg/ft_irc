#pragma once

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>

#include <string>
#include <vector>

using namespace std;

const int MAX_BUFFER_SIZE = 512;

class Client {
   public:
	class Close : public exception {
	   public:
		const char *what() const throw() {
			return " disconnected";
		}
	};

	Client();

	Client(const sockaddr_in &clientAddr);

	~Client();

	const char *Address() const;

	in_port_t Port() const;

	void PushBuffer(uint8_t *c, const size_t &size);

	string Identity() const;

	bool HasPendingCommand() const;

	bool IsValid() const;

	void SetPassword(bool value);

	bool HasSetPassword() const;

	void SetRegistered(bool value);

	bool IsRegistered() const;

	string &GetNickname();

	void SetNickname(const string &value);

	string &GetUser();

	void SetUser(vector<string> &values);

	string &GetAlias();

	void SetAlias(const string &value);

	vector<string> CreateArgs();

   private:
	vector<string> Split() const;

	string _buffer;
	sockaddr_in _client_addr;
	string _user;
	string _nickname;
	string _alias;
	string _fullname;
	string _mode;
	bool _has_set_pwd;
	bool _is_registered;
};

ostream &operator<<(ostream &stream, const Client &client);
