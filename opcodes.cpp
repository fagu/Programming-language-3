#include "opcodes.h"
#include "type.h"
#include "parseresult.h"

Type* binaryinputtype(OPCODE code) {
	if (code == AND || code == OR)
		return ParseRes->boolType;
	return ParseRes->intType;
}

Type* binaryresulttype(OPCODE code) {
	if (code == PLUS || code == MINUS || code == TIMES || code == DIV || code == MOD)
		return ParseRes->intType;
	return ParseRes->boolType;
}
