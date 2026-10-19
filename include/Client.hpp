#pragma once

#include <stdint.h>

#include <cstddef>
#include <map>
#include <string>
#include <vector>

class Client {
   public:
	Client();
	~Client();

	// Input buffering / parsing
	bool HasPendingCommand() const;
	void PushBuffer(uint8_t* c, const std::size_t& size);
	std::vector<std::string> CreateArgs();

	// Output buffering
	bool HasPendingOutput() const;
	void QueueOutput(const std::string& message);
	void ConsumeOutput(std::size_t size);
	const std::string& GetOutputBuffer() const;

	// Client identity
	void SetNickname(const std::string& value);
	void SetUser(const std::vector<std::string>& values);
	void SetHost(const std::string& host);
	const std::string& GetHost() const;
	std::string Identity() const;
	const std::string& GetNickname() const;
	const std::string& GetLower() const;
	std::string& GetUser();

	// Registration
	bool HasSetPassword() const;
	bool IsRegistered() const;
	void SetPassword(bool value);
	bool SetRegistered();

	// Closing state
	void Abort() throw();
	bool IsClosing() const;
	bool ShouldClose() const;
	void MarkClosing(const std::string& reason);
	void MarkDead(const std::string& reason);
	const std::string& QuitReason() const;

   private:
	enum ClosingState { OPEN, DRAIN, NOW };

	// Parsing
	std::vector<std::string> ParseLine(const std::string& line);

	// Input / output buffers
	std::string _buffer;
	std::string _output_buffer;

	// Identity
	std::string _host;
	std::string _user;
	std::string _nickname;
	std::string _lower_nickname;
	std::string _fullname;
	std::string _mode;

	// Registration
	bool _has_user;
	bool _has_set_pwd;
	bool _is_registered;

	// Closing state
	ClosingState _close_state;
	std::string _quit_reason;
};

typedef std::map<int, Client> ClientMap;
