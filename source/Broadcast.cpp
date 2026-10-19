#include <algorithm>

#include "Server.hpp"

using namespace std;

// Output queues / broadcasts
// [francais] Modification: centralise la mise en file, on ignore les destinataires absents
// et surveille seulement les files contenant des données.
void Server::QueueMessage(const string& message) {
	QueueMessage(_socket_in_use, message);
}

// [francais] Modification: centralise la mise en file, on ignore les destinataires absents
// et surveille seulement les files contenant des données.
void Server::QueueMessage(const int fd, const string& message) {
	ClientMap::iterator it = _clients.find(fd);
	if (it == _clients.end() || message.empty()) return;
	it->second.QueueOutput(message);
	if (it->second.HasPendingOutput()) EnablePollout(fd);
}

void Server::BroadcastMessages(const vector<int>& members,
                               const string& message) {
	for (size_t j = 0; j < members.size(); ++j) {
		QueueMessage(members[j], message);
	}
}

void Server::BroadcastMessages(const vector<int>& members,
                               const string& message, const int fd) {
	for (size_t j = 0; j < members.size(); ++j) {
		if (members[j] != fd) {
			QueueMessage(members[j], message);
		}
	}
}

void Server::BroadcastMessages(const vector<int>& members,
                               const string& message, vector<int>& notified) {
	for (vector<int>::const_iterator member = members.begin();
	     member != members.end(); ++member) {
		if (find(notified.begin(), notified.end(), *member) == notified.end()) {
			QueueMessage(*member, message);
			notified.push_back(*member);
		}
	}
}
