#include <stdio.h>
#include "opcodes.h"
#include "type.h"

string opnames[NUMBEROFOPCODES];

void init() {
#define INSTRUCTION(c,n,code) opnames[c]=string(#c);
#include "vminstructions.cpp"
#undef INSTRUCTION
	for (int i = 0; i < NUMBEROFOPCODES; i++) {
		if (!opnames[i].length()) {
			fprintf(stderr, "Opcode %d does not have a name!\n", i);
		}
	}
}

string opname(OPCODE op) {
	return opnames[op];
}
