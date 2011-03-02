#ifndef OPCODES_H
#define OPCODES_H

enum OPCODE {
	FUNC,
	FUNC_MAIN,
	HERE_STOP,
	ALLOC_STACK,
	INT_CONST,
	COPY_STACK,
	PRINT,
	ALLOC_HEAP_CONSTAMOUNT,
	ALLOC_HEAP_VARAMOUNT,
	SET_HEAP,
	GET_HEAP,
	SET_ARRAY,
	GET_ARRAY,
	JUMPIF,
	JUMP,
	CALL,
	DUMP_STACK,
	DUMP_HEAP,
	PLUS,
	MINUS,
	TIMES,
	DIV,
	MOD,
	EQUAL,
	LESS,
	GREATER,
	LESSOREQUAL,
	GREATEROREQUAL,
	UNEQUAL,
	AND,
	OR,
	NUMBEROFOPCODES // ATTENTION: This MUST be the last entry
};

#endif // OPCODES_H