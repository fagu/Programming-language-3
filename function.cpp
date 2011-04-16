#include "parseresult.h"
#include "function.h"
#include "instruction.h"
#include "type.h"

int FunctionDeclaration::find(Environment *e) {
	Environment *ne = new Environment;
	ne->addParent(e);
	if (env)
		ne->addParent(env);
	for (int i = 0; i < parameters->size(); i++) {
		(*parameters)[i]->find(ne);
	}
	DeclarationInstruction * returndec = new DeclarationInstruction(Location(), resulttype, new string("return")); // TODO use real location
	returndec->find(ne);
	instructions->find(ne);
	delete ne;
	return returndec->varpos;
}
