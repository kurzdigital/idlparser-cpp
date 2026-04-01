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

static int idl_is_subtype(const char *s) {
	return !strncmp(s, "DL", 2) || !strncmp(s, "ID", 2);
}

static const char *idl_subtype(const char *s, const char *end,
		char (*code)[3]) {
	for (const char *p = s; p < end; ++p) {
		if (!strchr(IDL_DIGITS, *p)) {
			continue;
		}
		const char *d = p;
		for (; d < end && strchr(IDL_DIGITS, *d); ++d);
		// Check if there are 8 or more consecutive digits.
		if (d - p < 8 || end - d < 3 || !idl_is_subtype(d)) {
			continue;
		}
		strncpy(*code, d, 2);
		d += 2;
		while (end - d > 2) {
			// Skip over everything that is not a "D" or "I".
			for (; d < end && !strchr("DI", *d); ++d);
			if (end - d > 2 && idl_is_subtype(d)) {
				// Return pointer after second "DL|ID".
				return d + 2;
			}
			++d;
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
		return ""; // There needs to be some white space after ANSI.
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
	const char *end = s.c_str() + s.size();
	size_t pos = s.find_first_not_of(IDL_WHITE_SPACE);
	const char *p = s.c_str() + (pos == std::string::npos ? 0 : pos);
	if (*p == '@') {
		char code[] = {0,0,0};
		p = idl_subtype(p, end, &code);
		if (*code) {
			idl_add(&idl, "DL", code);
		}
	}

	// Split and collect key/value pairs.
	bool skip_subtype = false;
	for (const char *start = p;
			// The loop needs to run after the last byte, too.
			p <= end;
			++p) {
		// Wait until control character (LF or CR).
		if (p < end && *p > 0x1f) {
			continue;
		}
		// Per spec, CR (0x0d) is the subfile boundary marker. The segment
		// immediately following a CR may begin with a 2-char subfile type
		// identifier (e.g. "ZV", "ZI") prepended to the first field.
		// Detect this by checking that the marker matches the key prefix
		// (jurisdiction fields always share their 2-char subfile prefix).
		const char *seg = start;
		if (skip_subtype && (p - start) >= 4 &&
				start[0] == start[2] && start[1] == start[3]) {
			seg = start + 2;
		}
		int vlen = (p - seg) - 3;
		if (vlen > 0 && strchr("DZ", *seg) &&
				strchr(IDL_LETTERS, seg[1]) &&
				strchr(IDL_LETTERS, seg[2])) {
			std::string key(seg, 3);
			std::string value(seg + 3, vlen);
			if (key == "DBC") {
				idl_resolve_sex(value);
			}
			idl_add(&idl, key, value);
		}
		skip_subtype = (p < end && *p == 0x0d);
		start = p + 1;
	}

	return idl;
}
#endif // IDL_PARSER_IMPLEMENTATION

#endif
