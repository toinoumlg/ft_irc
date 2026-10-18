#pragma once

#include <string>
#include <vector>

class Channel {
   public:
	Channel(const std::string& name, int creator_fd);
	Channel(const std::string& name, int creator_fd, const std::string& key);
	~Channel();

	// Name
	static bool IsValidName(const std::string& name);
	const std::string& GetName() const;
	const std::string& GetLower() const;

	// Members / capacity
	bool HasClient(int fd) const;
	bool IsEmpty() const;
	bool IsFull() const;
	void AddClient(int fd);
	void RemoveClient(int fd);
	void SetMaxClient(unsigned short max);
	const std::vector<int>& GetClients() const;
	unsigned int Limit() const;

	// Operators
	bool IsOperator(int fd) const;
	void AddOperator(int fd);
	void RemoveOperator(int fd);

	// Topic
	bool IsTopicRestricted() const;
	void SetTopic(const std::string& topic);
	void SetTopicRestricted(bool restricted);
	const std::string& GetTopic() const;

	// Invitations
	bool IsInviteOnly() const;
	bool IsInvited(int fd) const;
	void SetInviteOnly(bool enabled);
	void AddInvitation(int fd);
	void RemoveInvitation(int fd);

	// Channel key
	bool HasKey() const;
	bool CheckKey(const std::string& key) const;
	void SetKey(const std::string& key);

	// Mode summary
	void SetModes();
	std::string GetModes() const;

   private:
	// Name
	std::string _name;
	std::string _lower_name;

	// Members / capacity
	std::vector<int> _clients;
	unsigned short _max_client;

	// Operators
	std::vector<int> _operators;

	// Topic
	std::string _topic;
	bool _topic_restricted;

	// Invitations
	std::vector<int> _invited;
	bool _invite_only;

	// Channel key
	std::string _key;

	// Mode summary
	std::string _modes;
};
