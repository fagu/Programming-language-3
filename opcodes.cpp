#include "opcodes.h"
#include "type.h"
#include "parseresult.h"

Type* binaryresulttype(OPCODE code) {
	if (code == PLUS || code == MINUS || code == TIMES || code == DIV || code == MOD)
		return ParseRes->intType;
	return ParseRes->boolType;
}
