#pragma once

#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>

#include <map>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"

typedef std::map<int, Client> ClientMap;

class Irc {
   public:
	Irc(const char *port, const char *password);

	void Run();

	~Irc();

   private:
	typedef void (Irc::*CommandHandler)(std::vector<std::string> &, Client &);

	struct CommandEntry {
		const char *upper;
		const char *lower;
		CommandHandler handler;
	};

	static const CommandEntry _commands[];

	void RunCommand(Client &client);

	void ConnectClient();

	bool HandleRecv();

	void Cap(std::vector<std::string> &, Client &);

	void User(std::vector<std::string> &args, Client &client);

	void SendWelcome(Client &client) const;

	void Nick(std::vector<std::string> &args, Client &client);

	void Ping(std::vector<std::string> &args, Client &client);

	void Pass(std::vector<std::string> &args, Client &client);

	void Quit(std::vector<std::string> &, Client &);

	int _server_socket;
	int _socket_in_use;
	int _connected_clients;
	std::string _port;
	std::string _password;
	std::string _message;
	sockaddr_in _address;
	socklen_t _addrlen;
	std::vector<Channel> _channels;
	std::vector<pollfd> _pollfds;
	ClientMap _clients;
};
