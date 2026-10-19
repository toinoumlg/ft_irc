#include "Client.hpp"

#include <new>

#include "Helper.hpp"

using namespace std;

const int MAX_BUFFER_SIZE = 512;

// [francais] Modification: initialise la réception de USER, l'adresse du
// client.
Client::Client()
    : _host("127.0.0.1"),
      _nickname("*"),
      _has_user(false),
      _has_set_pwd(false),
      _is_registered(false),
      _close_state(OPEN) {}

Client::~Client() {}

// Input buffering / parsing
bool Client::HasPendingCommand() const {
	return _buffer.find('\n') != string::npos;
}

void Client::PushBuffer(uint8_t* c, const size_t& size) {
	_buffer.append(reinterpret_cast<char*>(c), size);

	const size_t last_end = _buffer.rfind('\n');
	const size_t pending_size = last_end == string::npos
	                                ? _buffer.size()
	                                : _buffer.size() - (last_end + 1);

	if (pending_size > MAX_BUFFER_SIZE - 1)
		MarkClosing("Input too long");
}

// [francais] Modification: ignore les lignes contenant NUL ou un CR interne après
// retrait de la terminaison, pour ne pas exécuter une commande mal formée.
vector<string> Client::CreateArgs() {
	const size_t line_end = _buffer.find('\n');

	if (line_end == string::npos)
		return vector<string>();

	if (line_end + 1 > MAX_BUFFER_SIZE) {
		MarkClosing("Input too long");
		return vector<string>();
	}

	string line = _buffer.substr(0, line_end);
	_buffer.erase(0, line_end + 1);

	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	if (line.find('\0') != string::npos || line.find('\r') != string::npos)
		return vector<string>();
	return ParseLine(line);
}

// Output buffering
bool Client::HasPendingOutput() const {
	return !_output_buffer.empty();
}

// [francais] Modification: borne la file a 4 Mio et abandonne la connexion saturée ou en
// échec d'allocation au lieu de laisser grossir la memoire sans limite.
void Client::QueueOutput(const string& message) {
	// [francais] Un destinataire lent ne doit pas faire grossir sans limite
	// la mémoire utilisée par le serveur pour sa file d'envoi.
	const size_t max_queue = 4 * 1024 * 1024;
	if (IsClosing()) return;
	if (message.size() > max_queue - _output_buffer.size()) {
		Abort();
		return;
	}
	try { _output_buffer += message; }
	catch (const std::bad_alloc&) { Abort(); }
}

void Client::ConsumeOutput(size_t size) {
	_output_buffer.erase(0, size);
}

const string& Client::GetOutputBuffer() const {
	return _output_buffer;
}

// Client identity
// [francais] Modification: conserve exactement le pseudo valide pour éviter les
// collisions créées par la troncature d'avant comme de 20 caractère identique et
// le 21ème différent. Avec troncature ils avaient du coup le même nickname.
void Client::SetNickname(const string& value) {
	_nickname = value;
	_lower_nickname = ToLower(_nickname);
}

// [francais] Modification: préserve le nom d'utilisateur valide et mémorise USER
// même lorsque le nom réel est vide.
void Client::SetUser(const vector<string>& values) {
	_user = values[0];
	_mode = values[1];
	_fullname = values[3];
	_has_user = true;
}

// [francais] Ajout: conserve l'adresse réelle du client au lieu de lui attribuer
// systématiquement localhost.
void Client::SetHost(const string& host) { _host = host; }

// [francais] Ajout: fournit l'adresse mémorisée pour vérifier la taille de
// l'identité et résoudre les destinations privées complètes.
const string& Client::GetHost() const { return _host; }

// [francais] Modification: construit nick!user@host avec l'adresse du client pour
// la bienvenue et les messages transmis.
string Client::Identity() const {
	return _nickname + '!' + _user + '@' + _host;
}

const string& Client::GetNickname() const {
	return _nickname;
}

const string& Client::GetLower() const {
	return _lower_nickname;
}

string& Client::GetUser() {
	return _user;
}

// Registration
bool Client::HasSetPassword() const {
	return _has_set_pwd;
}

bool Client::IsRegistered() const {
	return _is_registered;
}

void Client::SetPassword(const bool value) {
	_has_set_pwd = value;
}

// [francais] Modification: distingue USER absent d'un nom réel vide et autorise
// l'inscription lorsque PASS, NICK et USER sont réunis.
bool Client::SetRegistered() {
	if (_nickname != "*" && !_nickname.empty() && _has_set_pwd &&
	    _has_user)
		_is_registered = true;
	return _is_registered;
}

// Closing state
// [francais] Ajout: abandonne la connexion et libere ses tampons sans nouvelle
// allocation pour récupérer d'une saturation ou d'un manque de mémoire.
void Client::Abort() throw() {
	_close_state = NOW;
	_quit_reason.clear();
	string().swap(_buffer);
	string().swap(_output_buffer);
}

bool Client::IsClosing() const {
	return _close_state != OPEN;
}

bool Client::ShouldClose() const {
	return _close_state == NOW ||
	       (_close_state == DRAIN && _output_buffer.empty());
}

void Client::MarkClosing(const string& reason) {
	if (_close_state != OPEN)
		return;
	_close_state = DRAIN;
	_quit_reason = reason;
}

void Client::MarkDead(const string& reason) {
	if (_close_state == OPEN)
		_quit_reason = reason;
	_close_state = NOW;
}

const string& Client::QuitReason() const {
	return _quit_reason;
}

// Parsing
// [francais] Modification: limite le découpage à quinze paramètres et ignore un
// préfixe qui ne correspond pas au pseudo de la connexion.
vector<string> Client::ParseLine(const string& line) {
	vector<string> result;
	size_t pos = 0;

	while (pos < line.size() && line[pos] == ' ') ++pos;

	if (pos < line.size() && line[pos] == ':') {
		const size_t start = ++pos;
		while (pos < line.size() && line[pos] != ' ') ++pos;
		if (ToLower(line.substr(start, pos - start)) != _lower_nickname)
			return result;
		while (pos < line.size() && line[pos] == ' ') ++pos;
	}

	while (pos < line.size()) {
		if (result.size() == 15 || line[pos] == ':') {
			result.push_back(line.substr(pos + (line[pos] == ':' ? 1 : 0)));
			break;
		}

		const size_t end = line.find(' ', pos);

		if (end == string::npos) {
			result.push_back(line.substr(pos));
			break;
		}

		result.push_back(line.substr(pos, end - pos));
		pos = end + 1;

		while (pos < line.size() && line[pos] == ' ') ++pos;
	}

	return result;
}
