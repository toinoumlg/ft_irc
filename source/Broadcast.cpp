#include <algorithm>

#include "Server.hpp"

void Server::BroadcastMessages(const vector<int> &members,
                               const std::string &message) {
	for (size_t j = 0; j < members.size(); ++j) {
		QueueMessage(members[j], message);
	}
}

void Server::BroadcastMessages(const vector<int> &members,
                               const std::string &message, const int fd) {
	for (size_t j = 0; j < members.size(); ++j) {
		if (members[j] != fd) {
			QueueMessage(members[j], message);
		}
	}
}

void Server::BroadcastMessages(const vector<int> &members,
                               const std::string &message,
                               vector<int> &notified) {
	for (vector<int>::const_iterator member = members.begin();
	     member != members.end(); ++member) {
		if (find(notified.begin(), notified.end(), *member) == notified.end()) {
			QueueMessage(*member, message);
			notified.push_back(*member);
		}
	}
}

void Server::QueueMessage(const int fd, const string &message) {
	_clients[fd].QueueOutput(message);
	EnablePollout(fd);
}

void Server::QueueMessage(const string &message) {
	_clients[_socket_in_use].QueueOutput(message);
	EnablePollout(_socket_in_use);
}
