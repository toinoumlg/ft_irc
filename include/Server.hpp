#pragma once

#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>

#include <map>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"
#include "Response.hpp"
#include "StatusCode.hpp"

typedef map<int, Client> ClientMap;

class Server {
   public:
	Server(const char *port, const char *password);

	void Run();

	void ProcessClient(short client_event, int &i);

	~Server();

   private:
	typedef void (Server::*CommandHandler)(vector<string> &, Client &);

	struct CommandEntry {
		const char *upper;
		const char *lower;
		CommandHandler handler;
	};

	static const CommandEntry _commands[];

	void RunCommand(Client &client);

	void ConnectClient();

	void HandleRecv();

	void Cap(vector<string> &, Client &);

	void User(vector<string> &args, Client &client);

	void Nick(vector<string> &args, Client &client);

	static bool HasInvalidChar(const string &str);

	bool NicknameExists(const string &nickname);

	void Ping(vector<string> &, Client &);

	void Pass(vector<string> &args, Client &client);

	void Privmsg(vector<string> &args, Client &client);

	void Quit(vector<string> &, Client &);

	void EnablePollout(int fd);
	void QueueMessage(int fd, const string &message);
	void HandleSend();
	void DisablePollout(int fd);

	int _server_socket;
	int _socket_in_use;
	int _connected_clients;
	string _port;
	string _password;
	string _message;
	sockaddr_in _address;
	socklen_t _addrlen;
	vector<Channel> _channels;
	vector<pollfd> _pollfds;
	ClientMap _clients;
};
