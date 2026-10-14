#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"
#include "Response.hpp"

class Server {
   public:
	Server(const string &port, const string &password);

	void Run();

	static bool CheckPort(const char *port);

	void ProcessClient(short client_event, int &i);
	~Server();

   private:
	typedef void (Server::*CommandHandler)(vector<string> &, Client &);

	struct CommandEntry {
		const char *name;
		bool needsRegistration;
		CommandHandler handler;
	};

	static const CommandEntry _commands[];

	void RunCommand(Client &client);

	void ConnectClient();

	void HandleRecv(Client &client) const;

	void Cap(vector<string> &, Client &);

	void User(vector<string> &args, Client &client);

	void Nick(vector<string> &args, Client &client);

	bool NicknameExists(const string &nickname, const Client &client);

	void Ping(vector<string> &args, Client &client);

	void Pass(vector<string> &args, Client &client);

	void Privmsg(vector<string> &args, Client &client);
	void BroadcastMessages(const vector<int> &members, const string &message);
	void BroadcastMessages(const vector<int> &members,
	                       const std::string &message, int fd);
	void BroadcastMessages(const vector<int> &members,
	                       const std::string &message, vector<int> &notified);

	void Join(vector<string> &args, Client &client);

	void Part(vector<string> &args, Client &client);

	void Kick(vector<string> &args, Client &client);

	void Quit(vector<string> &, Client &client);

	void Topic(vector<string> &args, Client &client);
	void Invite(vector<string> &args, Client &client);
	void Mode(vector<string> &args, Client &client);
	void SendChannelTopic(Channel &channel, Client &client);

	void SendChannelNames(Channel &channel, Client &client);
	static vector<string> SplitTargets(string &args);
	static vector<pair<string, string> > SplitChannelsAndKeys(
	    vector<string> &args);
	void RemoveClientFromChannels();
	void RemoveClient(int &i);
	void NotifyWelcome();
	void NotifyQuit();
	vector<Channel>::iterator GetChannelByName(const string &name);
	int GetClientByNick(const string &nickname);

	void EnablePollout(int fd);
	void QueueMessage(const string &message);
	void QueueMessage(int fd, const string &message);
	void HandleSend(Client &client);
	void DisablePollout(int fd);
	void DisablePollin(int fd);

	int _server_socket;
	int _socket_in_use;
	int _connected_clients;
	string _port;
	string _password;
	string _message;
	vector<Channel> _channels;
	vector<pollfd> _pollfds;
	ClientMap _clients;
};
