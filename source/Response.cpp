#include "Response.hpp"

#include <vector>

#include "Client.hpp"

using namespace std;

// Wire format constants
const string Response::CR_LF = "\r\n";
const string Response::LOCALHOST = "127.0.0.1";

// Message formatting
// [francais] Modification: fait passer la réponse construite par Finish pour
// controler sa taille avant l'ajout de CRLF.
string Response::Build(const string& command) {
	return Finish(':' + LOCALHOST + ' ' + command);
}

// [francais] Modification: fait passer la réponse construite par Finish pour
// controler sa taille avant l'ajout de CRLF.
string Response::Build(const string& command, const string& message) {
	return Finish(':' + LOCALHOST + ' ' + command + ' ' + message);
}

static string CodeToString(const StatusCode code) {
	char str[4];
	str[0] = static_cast<char>('0' + code / 100);
	str[1] = static_cast<char>('0' + code / 10 % 10);
	str[2] = static_cast<char>('0' + code % 10);
	str[3] = '\0';
	return str;
}

/// [francais] Modification: fait passer la réponse construite par Finish pour
/// controler
// sa taille avant l'ajout de CRLF.
string Response::Build(const StatusCode code, const Client& client,
                       const string& message) {
	return Finish(':' + LOCALHOST + ' ' + CodeToString(code) + ' ' +
	              client.GetNickname() + ' ' + message);
}

// [francais] Modification: utilise l'identité complète du client et réduit le
// préfixe au pseudo si les paramètres structurels ne tiennent pas avec
// user@host.
string Response::Build(const Client& from, const string& command,
                       const string& message) {
	string prefix = ':' + from.Identity() + ' ' + command + ' ';
	const size_t trailing =
	    message.empty() || message[0] == ':' ? 0 : message.find(" :");
	const size_t structural =
	    trailing == string::npos ? message.size() : trailing + 2;
	// [francais] La partie utilisateur/hôte du préfixe est facultative selon la
	// RFC 2812, section 2.3.1.
	if (prefix.size() + structural > 510)
		prefix = ':' + from.GetNickname() + ' ' + command + ' ';
	return Finish(prefix + message);
}

static string ErrorParameter(const string& value) {
	if (value.empty() || value[0] == ':' ||
	    value.find_first_of(" \r\n") != string::npos ||
	    value.find('\0') != string::npos)
		return "*";
	return value;
}

static string FormatError(StatusCode code, const Client& client,
                          vector<string> parameters, const string& description) {
	const string prefix = ':' + Response::LOCALHOST + ' ' + CodeToString(code) +
	                      ' ' + client.GetNickname();
	const size_t available = 510 - prefix.size() - parameters.size() - 2;
	const string text = description.substr(0, available - parameters.size());
	const size_t budget = available - text.size();
	for (size_t i = 0; i < parameters.size(); ++i)
		parameters[i] = ErrorParameter(parameters[i]);

	// Shorten only the diagnostic copy, preserving each parameter boundary.
	for (;;) {
		size_t total = 0;
		size_t longest = 0;
		for (size_t i = 0; i < parameters.size(); ++i) {
			total += parameters[i].size();
			if (parameters[i].size() > parameters[longest].size()) longest = i;
		}
		if (total <= budget) break;
		const size_t removable = parameters[longest].size() - 1;
		const size_t excess = total - budget;
		parameters[longest].resize(parameters[longest].size() -
		    (excess < removable ? excess : removable));
	}
	string line = prefix;
	for (size_t i = 0; i < parameters.size(); ++i) line += ' ' + parameters[i];
	return line + " :" + text + Response::CR_LF;
}

string Response::BuildError(StatusCode code, const Client& client,
                            const string& target, const string& description) {
	return FormatError(code, client, vector<string>(1, target), description);
}

string Response::BuildError(StatusCode code, const Client& client,
                            const string& first, const string& second,
                            const string& description) {
	vector<string> parameters;
	parameters.push_back(first);
	parameters.push_back(second);
	return FormatError(code, client, parameters, description);
}

// [francais] Ajout: limite la ligne a 512 octets avec CRLF ; raccourcit le
// dernier texte si possible, sinon omet la ligne plutot que couper sa
// structure. ce que fait la plupart des autres server IRC
string Response::Finish(const string& line) {
	if (line.size() <= 510)
		return line + CR_LF;
	const size_t trailing = line.find(" :");
	// [francais] Ne tronque jamais les noms de commandes, les destinations ni
	// les autres paramètres structurels.
	if (trailing == string::npos || trailing + 2 > 510)
		return "";
	return line.substr(0, 510) + CR_LF;
}
