#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>

#include <exception>
#include <ostream>
#include <string>
#include <vector>

using namespace std;

const int MAX_BUFFER_SIZE = 512;

class Client {
	private:
		string _buffer;
		sockaddr_in _client_addr;

		string _user;
		string _nickname;
		string _alias;
		string _fullname;
		string _mode;

		bool _has_set_pwd;
		bool _is_registered;

		vector<string> ParseLine(const string &line) const;

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

		// Connection
		const char *Address() const;
		in_port_t Port() const;

		// IRC buffer / parsing
		void PushBuffer(uint8_t *c, const size_t &size);
		bool HasPendingCommand() const;
		vector<string> CreateArgs();

		// Client identity
		string Identity() const;

		string &GetNickname();
		void SetNickname(const string &value);

		string &GetUser();
		void SetUser(vector<string> &values);

		string &GetAlias();
		void SetAlias(const string &value);

		// Registration
		void SetPassword(bool value);
		bool HasSetPassword() const;

		void SetRegistered(bool value);
		bool IsRegistered() const;
};

ostream &operator<<(ostream &stream, const Client &client);