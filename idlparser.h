#ifndef __IDLParser_h__
#define __IDLParser_h__

#include <string>
#include <map>
#include <vector>

struct IDL {
	std::string iin;
	std::map<std::string, std::string> elements;
	std::vector<std::string> order;
};

IDL parse_idl(const std::string &);

#ifdef IDL_PARSER_IMPLEMENTATION
#define IDL_WHITE_SPACE "\t\r\n "
#define IDL_DIGITS "0123456789"
#define IDL_LETTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

static std::string idl_trim(const std::string &s) {
	size_t start = s.find_first_not_of(IDL_WHITE_SPACE);
	size_t end = s.find_last_not_of(IDL_WHITE_SPACE);
	if (start == std::string::npos || end == std::string::npos) {
		return "";
	}
	return s.substr(start, end - start + 1);
}

static void idl_add(IDL *idl, const std::string &key, const std::string &value) {
	idl->elements[key] = idl_trim(value);
	idl->order.push_back(key);
}

static void idl_resolve_sex(std::string &value) {
	if (value == "1") {
		value = "M";
	} else if (value == "2") {
		value = "F";
	} else if (value == "9") {
		value = "X";
	}
}

static const char *idl_subtype(const char *s, unsigned int left,
		char (*code)[3]) {
	for (const char *p = s; left > 0; --left, ++p) {
		if (!strchr(IDL_DIGITS, *p)) {
			continue;
		}
		const char *d = p;
		for (; left > 0 && strchr(IDL_DIGITS, *d); --left, ++d);
		// Check if there are 8 or more consecutive digits.
		if (d - p < 8 || left < 3 ||
				// Check if this is followed by "DL" or "ID".
				(strncmp(d, "DL", 2) && strncmp(d, "ID", 2))) {
			continue;
		}
		strncpy(*code, d, 2);
		d += 2;
		while (left > 2) {
			// Skip over everything that is not a "D" or "I".
			for (; left > 0 && !strchr("DI", *d); --left, ++d);
			if (left > 2 &&
					// Check if it is a "DL" or "ID".
					(!strncmp(d, "DL", 2) || !strncmp(d, "ID", 2))) {
				// Return pointer after second "DL|ID".
				return d + 2;
			}
		}
	}
	return s;
}

static std::string idl_find_iin(const std::string &s, unsigned int *size) {
	size_t pos = s.find("ANSI");
	if (pos == std::string::npos) {
		return "";
	}
	pos += 4;
	pos = s.find_first_not_of(IDL_WHITE_SPACE, pos);
	if (pos == std::string::npos) {
		return ""; // There need to be some white space after ANSI.
	}
	// The IIN is at most 6 digits long.
	size_t len = s.size();
	size_t start = pos;
	for (int n = 6; n > 0 && pos < len && strchr(IDL_DIGITS, s[pos]);
			--n, ++pos);
	*size = pos - start;
	return *size > 0 ? s.substr(start, *size) : "";
}

IDL parse_idl(const std::string &s) {
	IDL idl;
	if (s.empty()) {
		return idl;
	}

	// Try to find the Issuer Identification Number (IIN).
	{
		unsigned int size = 0;
		std::string iin = idl_find_iin(s, &size);
		if (!iin.empty()) {
			idl.iin = iin;
		}
	}

	// Check for sub file pattern.
	const char *p = s.c_str();
	{
		int left = s.size();
		for (; left > 0 && strchr(IDL_WHITE_SPACE, *p); --left, ++p);
		if (*p == '@') {
			char code[] = {0,0,0};
			p = idl_subtype(p, left, &code);
			if (*code) {
				idl_add(&idl, "DL", code);
			}
		}
	}

	// Split and collect key/value pairs.
	const char *start = p;
	for (size_t i = s.size(); i > 0; --i, ++p) {
		if (*p > 0x1f) { // Check for control characters.
			continue;
		}
		int vlen = (p - start) - 3;
		if (vlen > 0 && strchr("DZ", *start) &&
				strchr(IDL_LETTERS, *(start + 1)) &&
				strchr(IDL_LETTERS, *(start + 2))) {
			std::string key(start, 3);
			std::string value(start + 3, vlen);
			if (key == "DBC") {
				idl_resolve_sex(value);
			}
			idl_add(&idl, key, value);
		}
		start = p + 1;
	}

	return idl;
}
#endif // IDL_PARSER_IMPLEMENTATION

#endif
