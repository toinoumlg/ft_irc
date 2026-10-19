#include "Helper.hpp"

#include <cctype>

using namespace std;

// IRC name normalization
string ToLower(const string& str) {
	string out = string(str);
	for (size_t i = 0; i < out.size(); ++i) {
		const unsigned char c = static_cast<unsigned char>(out[i]);
		if (c >= 'A' && c <= 'Z')
			out[i] = c + 32;
		if (c == '[')
			out[i] = '{';
		if (c == ']')
			out[i] = '}';
		if (c == '\\')
			out[i] = '|';
		if (c == '~')
			out[i] = '^';
	}
	return out;
}

// Identity validation / sanitization
// [francais] Modification: des plages ASCII des pseudos pour m'y retrouver.
static bool IsValidChar(unsigned char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
	       (c >= '0' && c <= '9') || c == '[' || c == '\\' || c == ']' || c == '^' ||
	       c == '_' || c == '-' || c == '{' || c == '}' || c == '|' || c == '`';
}

// [francais] Modification: refuse les pseudos de plus de neuf caracteres avant
// leur stockage, au lieu de les accepter puis les tronquer. ça évite tellement de problème
// de respecter la rfc.
bool IsValidNickname(const string& str) {
	if (str.empty() || str.size() > 9 || isdigit(static_cast<unsigned char>(str[0])) ||
	    str[0] == '-' || str[0] == '/')
		return false;

	for (size_t i = 0; i < str.size(); ++i) {
		const unsigned char c = static_cast<unsigned char>(str[i]);
		if (!IsValidChar(c))
			return false;
	}
	return true;
}

string ToValidUsername(const string& str) {
	string out;
	for (size_t i = 0; i < str.size(); ++i) {
		if (IsValidChar(static_cast<unsigned char>(str[i])))
			out += str[i];
	}
	return out.empty() ? "user" : out.substr(0, 20);
}

// Protocol parameter validation
// [francais] Ajout: vérifie les caract du nom d'utilisateur sans lui appliquer le
// filtrage ni la troncature des pseudos.
bool IsValidUsername(const string& str) {
	return !str.empty() && str.find_first_of(" \r\n@") == string::npos &&
	       str.find('\0') == string::npos;
}

// [francais] Ajout: refus des clés vides, trop longues ou contenant les octets exclus
// par la rfc retenue pour éviter des clés mal formées.
bool IsValidKey(const string& str) {
	if (str.empty() || str.size() > 23) return false;
	for (size_t i = 0; i < str.size(); ++i) {
		const unsigned char c = static_cast<unsigned char>(str[i]);
		// Conservative interpretation of RFC 2812 key / erratum 4836.
		if (c == 0 || c == 6 || c == 9 || c == 10 || c == 11 || c == 12 || c == 13 ||
		    c == 32 || c > 127) return false;
	}
	return true;
}
