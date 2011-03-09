#include <stdio.h>
#include <assert.h>
#include "opcodes.h"
#include "type.h"

string opnames[NUMBEROFOPCODES];
bool opconsts[NUMBEROFOPCODES];

void init() {
#define INSTRUCTION(c,n,const,code) opnames[c]=string(#c);
#include "vminstructions.cpp"
#undef INSTRUCTION
#define INSTRUCTION(c,n,const,code) opconsts[c]=const;
#include "vminstructions.cpp"
#undef INSTRUCTION
	for (int i = 0; i < NUMBEROFOPCODES; i++) {
		assert(opnames[i].length());
	}
}

string opname(OPCODE op) {
	return opnames[op];
}

bool opconst(OPCODE op) {
	return opconsts[op];
}
