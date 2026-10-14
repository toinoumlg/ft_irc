#include "Helper.hpp"

string ToLower(const string &str) {
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

static bool IsValidChar(unsigned char c) {
	return isalnum(c) || c == '[' || c == '\\' || c == ']' || c == '^' ||
	       c == '_' || c == '-' || c == '{' || c == '}' || c == '|' || c == '`';
}

bool IsValidNickname(const string &str) {
	if (str.empty() || isdigit(static_cast<unsigned char>(str[0])) ||
	    str[0] == '-' || str[0] == '/')
		return false;

	for (size_t i = 0; i < str.size(); ++i) {
		const unsigned char c = static_cast<unsigned char>(str[i]);
		if (!IsValidChar(c))
			return false;
	}
	return true;
}

string ToValidUsername(const string &str) {
	string out;
	for (size_t i = 0; i < str.size(); ++i) {
		if (IsValidChar(static_cast<unsigned char>(str[i])))
			out += str[i];
	}
	return out.empty() ? "user" : out.substr(0, 20);
}