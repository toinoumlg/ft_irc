#pragma once

#include <string>
#include <vector>

class Channel {
   private:
	std::string _name;
	std::vector<int> _clients;
	std::vector<int> _operators;
	std::string _topic;
	bool _topic_restricted;
	std::vector<int> _invited;
	bool _invite_only;

   public:
	Channel(const std::string& name, int creator_fd);
	~Channel();

	static bool IsValidName(const std::string& name);

	const std::string& GetName() const;

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
};
