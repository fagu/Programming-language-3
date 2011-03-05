#ifndef OPCODES_COMPILER_H
#define OPCODES_COMPILER_H
#include "opcodes.h"

class Type;

Type * binaryinputtype(OPCODE code);
Type * binaryresulttype(OPCODE code);

#endif // OPCODES_COMPILER_H