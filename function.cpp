#include "parseresult.h"
#include "function.h"
#include "instruction.h"
#include "type.h"

int FunctionDeclaration::find(Environment *e) {
	e->enterBlock();
	for (int i = 0; i < parameters->size(); i++) {
		(*parameters)[i]->find(e);
	}
	DeclarationInstruction * returndec = new DeclarationInstruction(Location(), resulttype, new string("return")); // TODO use real location
	returndec->find(e);
	instructions->find(e);
	e->exitBlock();
	return returndec->pos;
}
