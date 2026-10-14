#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "Helper.hpp"

class Channel {
   private:
	std::string _name;
	std::string _lower_name;
	std::string _key;
	std::string _modes;
	std::vector<int> _clients;
	std::vector<int> _operators;
	std::string _topic;
	bool _topic_restricted;
	std::vector<int> _invited;
	bool _invite_only;
	unsigned short _max_client;

   public:
	Channel(const std::string& name, int creator_fd);
	Channel(const std::string& name, int creator_fd, const std::string& key);
	~Channel();

	static bool IsValidName(const std::string& name);

	const std::string& GetName() const;
	const std::string& GetLower() const;

	bool HasClient(int fd) const;
	void AddClient(int fd);
	void RemoveClient(int fd);
	const std::vector<int>& GetClients() const;

	bool IsOperator(int fd) const;
	void AddOperator(int fd);
	void RemoveOperator(int fd);

	bool IsEmpty() const;

	const std::string& GetTopic() const;
	void SetTopic(const std::string& topic);

	bool IsTopicRestricted() const;
	void SetTopicRestricted(bool restricted);

	bool IsInviteOnly() const;
	void SetInviteOnly(bool enabled);
	void AddInvitation(int fd);
	bool IsInvited(int fd) const;
	void RemoveInvitation(int fd);

	bool HasKey() const;
	void SetKey(const std::string& key);
	bool CheckKey(const std::string& key) const;

	void SetMaxClient(unsigned short max);
	bool IsFull() const;
	unsigned int Limit() const;

	void SetModes();
	std::string GetModes() const;
};
