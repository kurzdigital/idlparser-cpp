#define IDL_PARSER_IMPLEMENTATION
#include "idlparser.h"

#include <iostream>

int main() {
	std::string input;
	for (std::string line; std::getline(std::cin, line); ) {
		input += line + '\n';
	}

	IDL idl = parse_idl(input);
	std::cout << "IIN:[" << idl.iin << "]" << std::endl;
	for (const auto &element : idl.elements) {
		std::cout
			<< element.first
			<< ":[" << element.second << "]"
			<< std::endl;
	}

	return 0;
}
