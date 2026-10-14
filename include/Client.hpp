#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>

#include <exception>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "Helper.hpp"

using namespace std;

const int MAX_BUFFER_SIZE = 512;

class Client {
   private:
	enum ClosingState { OPEN, DRAIN, NOW };
	string _buffer;
	string _output_buffer;

	string _user;
	string _nickname;
	string _lower_nickname;
	string _alias;
	string _fullname;
	string _mode;

	bool _has_set_pwd;
	bool _is_registered;
	ClosingState _close_state;
	string _quit_reason;

	static vector<string> ParseLine(const string &line);

   public:
	Client();
	~Client();

	// IRC buffer / parsing
	void PushBuffer(uint8_t *c, const size_t &size);
	bool HasPendingCommand() const;
	vector<string> CreateArgs();
	void QueueOutput(const string &message);
	bool HasPendingOutput() const;
	const string &GetOutputBuffer() const;
	void ConsumeOutput(size_t size);

	// Client identity
	string Identity() const;

	const string &GetNickname() const;
	const string &GetLower() const;
	void SetNickname(const string &value);

	string &GetUser();
	void SetUser(const vector<string> &values);

	string &GetAlias();
	void SetAlias(const string &value);

	// Registration
	void SetPassword(bool value);
	bool HasSetPassword() const;
	bool SetRegistered();
	bool IsRegistered() const;

	// Closing state
	bool IsClosing() const;
	void MarkClosing(const string &reason);
	void MarkDead(const string &reason);
	bool ShouldClose() const;
	const string &QuitReason() const;
};

typedef map<int, Client> ClientMap;
