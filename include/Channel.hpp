#pragma once

#include <string>
#include <vector>

class Channel {
	private:
		std::string _name;
		std::vector<int> _clients;
		std::vector<int> _operators;

	public:
		Channel(const std::string &name, int creator_fd);
		~Channel();

		const std::string &GetName() const;

		bool HasClient(int fd) const;
		void AddClient(int fd);
		void RemoveClient(int fd);
		const std::vector<int> &GetClients() const;

		bool IsOperator(int fd) const;
		void AddOperator(int fd);
		void RemoveOperator(int fd);

		bool IsEmpty() const;
};