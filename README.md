# IDL Parser for C++

Single-file header library to parse an
[International Driver License][idl] (IDL).

This is a C++-only alternative to [idlparser-c][cimp], which uses
`std::string` for automatic memory managment.

## Barcode

IDLs are usually encoded in a [PDF417][pdf417] barcode.
Reading this barcode is not part of this library.

## How to use

Before you include the header file in *one* C++ source file, you need to
define `IDL_PARSER_IMPLEMENTATION` to create the implementation:

	#define IDL_PARSER_IMPLEMENTATION
	#include "idlparser.h"

## How to parse an IDL

Then invoke `parse_idl()` with IDL data:

	IDL idl = parse_idl(data);
	if (idl.count < 1) {
		std::cerr << "error: failed to parse IDL" << std::endl;
	}

Now the structure `idl` contains the parsed elements of the IDL:

	std::cout << "IIN: " << idl.iin << std::endl;

[cimp]: https://github.com/kurzdigital/idlparser-c
[idl]: http://www.aamva.org/DL-ID-Card-Design-Standard/
[pdf417]: https://en.wikipedia.org/wiki/PDF417
