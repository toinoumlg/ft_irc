#include "Server.hpp"

#include "Helper.hpp"
#include "Response.hpp"

using namespace std;

// Lookup / argument parsing
vector<Channel>::iterator Server::GetChannelByName(const string& name) {
	const string lower = ToLower(name);
	for (vector<Channel>::iterator it = _channels.begin();
	     it != _channels.end(); ++it)
		if (it->GetLower() == lower)
			return it;
	return _channels.end();
}

// [francais] Ajout: interprète les destinations privées avec
// userr, hote ou serveur et distingue une cible absente d'une cible
// j'ai le mot sur le bout de la langue mais il ne me revient pas
// en gros pas claire.
int Server::GetMessageTarget(const string& target) {
	const size_t bang = target.find('!');
	const size_t at = target.rfind('@');
	const size_t percent = target.find('%');
	if (bang != string::npos && at != string::npos && bang < at) {
		const int fd = GetClientByNick(target.substr(0, bang));
		if (fd >= 0) {
			Client& c = _clients.find(fd)->second;
			if (c.GetUser() == target.substr(bang + 1, at - bang - 1) &&
			    c.GetHost() == target.substr(at + 1))
				return fd;
		}
	}
	if (at == string::npos && percent == string::npos)
		return GetClientByNick(target);
	if (at != string::npos && target.substr(at + 1) != Response::LOCALHOST)
		return -1;
	const size_t end = at == string::npos ? target.size() : at;
	if (percent != string::npos && percent >= end)
		return -1;
	const string user =
	    target.substr(0, percent == string::npos ? end : percent);
	const string host = percent == string::npos
	                        ? ""
	                        : target.substr(percent + 1, end - percent - 1);
	int found = -1;
	for (ClientMap::iterator it = _clients.begin(); it != _clients.end();
	     ++it) {
		if (!it->second.IsRegistered() || it->second.GetUser() != user ||
		    (percent != string::npos && it->second.GetHost() != host))
			continue;
		if (found != -1)
			return -2;
		found = it->first;
	}
	return found;
}

int Server::GetClientByNick(const string& nickname) {
	const string lower = ToLower(nickname);
	for (ClientMap::const_iterator it = _clients.begin(); it != _clients.end();
	     ++it) {
		if (it->second.IsRegistered() && it->second.GetLower() == lower)
			return it->first;
	}
	return -1;
}

vector<string> Server::SplitTargets(string& args) {
	vector<string> targets;
	while (args.begin() != args.end()) {
		const size_t pos = args.find(',');
		if (pos == string::npos) {
			targets.push_back(args);
			break;
		}
		targets.push_back(args.substr(0, pos));
		args.erase(0, pos + 1);
	}
	return targets;
}

vector<pair<string, string> > Server::SplitChannelsAndKeys(
    vector<string>& args) {
	vector<pair<string, string> > channels;

	string& names = args[0];
	while (names.begin() != names.end()) {
		const size_t pos = names.find(',');
		if (pos == string::npos) {
			channels.push_back(pair<string, string>(names, ""));
			break;
		}
		channels.push_back(pair<string, string>(names.substr(0, pos), ""));
		names.erase(0, pos + 1);
	}
	if (args.size() == 1)
		return channels;

	string& keys = args[1];
	vector<pair<string, string> >::iterator it = channels.begin();
	while (keys.begin() != keys.end() && it != channels.end()) {
		const size_t pos = keys.find(',');
		if (pos == string::npos) {
			it->second = keys;
			break;
		}
		it->second = keys.substr(0, pos);
		keys.erase(0, pos + 1);
		++it;
	}

	return channels;
}
