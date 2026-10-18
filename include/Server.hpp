#pragma once

#include <poll.h>

#include <string>
#include <utility>
#include <vector>

#include "Channel.hpp"
#include "Client.hpp"

class Server {
   public:
	Server(const std::string& port, const std::string& password);
	~Server();

	// Startup
	static bool CheckPort(const char* port);
	void Run();

   private:
	// Command dispatch
	typedef void (Server::*CommandHandler)(std::vector<std::string>&, Client&);
	struct CommandEntry {
		const char* name;
		bool needsRegistration;
		CommandHandler handler;
	};
	static const CommandEntry _commands[];
	void RunCommand(Client& client);

	// Network I/O
	void ConnectClient();
	void ProcessClient(short client_event, int& i);
	void HandleRecv(Client& client) const;
	void HandleSend(Client& client);
	void EnablePollout(int fd);
	void DisablePollout(int fd);
	void DisablePollin(int fd);

	// Connection commands
	void Cap(std::vector<std::string>& args, Client& client);
	void Pass(std::vector<std::string>& args, Client& client);
	void Nick(std::vector<std::string>& args, Client& client);
	void User(std::vector<std::string>& args, Client& client);
	void Ping(std::vector<std::string>& args, Client& client);
	void Quit(std::vector<std::string>& args, Client& client);

	// Messaging / channel commands
	void Privmsg(std::vector<std::string>& args, Client& client);
	void Join(std::vector<std::string>& args, Client& client);
	void Part(std::vector<std::string>& args, Client& client);
	void Kick(std::vector<std::string>& args, Client& client);
	void Topic(std::vector<std::string>& args, Client& client);
	void Invite(std::vector<std::string>& args, Client& client);
	void Mode(std::vector<std::string>& args, Client& client);

	// Lookup / argument parsing
	bool NicknameExists(const std::string& nickname, const Client& client);
	std::vector<Channel>::iterator GetChannelByName(const std::string& name);
	int GetClientByNick(const std::string& nickname);
	static std::vector<std::string> SplitTargets(std::string& args);
	static std::vector<std::pair<std::string, std::string> >
	SplitChannelsAndKeys(std::vector<std::string>& args);

	// Output queues / broadcasts
	void QueueMessage(const std::string& message);
	void QueueMessage(int fd, const std::string& message);
	void BroadcastMessages(const std::vector<int>& members,
	                       const std::string& message);
	void BroadcastMessages(const std::vector<int>& members,
	                       const std::string& message, int fd);
	void BroadcastMessages(const std::vector<int>& members,
	                       const std::string& message,
	                       std::vector<int>& notified);

	// Replies / notifications
	void SendChannelTopic(Channel& channel, Client& client);
	void SendChannelNames(Channel& channel, Client& client);
	void NotifyWelcome();
	void NotifyQuit();

	// Disconnection / cleanup
	void RemoveClientFromChannels();
	void RemoveClient(int& i);

	// Network state
	int _server_socket;
	int _socket_in_use;
	int _connected_clients;

	// Configuration
	std::string _port;
	std::string _password;

	// Channels, monitored sockets and clients
	std::vector<Channel> _channels;
	std::vector<pollfd> _pollfds;
	ClientMap _clients;
};
