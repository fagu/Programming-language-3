#include "parseresult.h"
#include "function.h"
#include "instruction.h"
#include "type.h"

int FunctionDefinition::find(Environment *e) {
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

TypePointerFunction* FunctionDefinition::funcType() {
	vector<TypePointer*> *argTypes = new vector<TypePointer*>;
	for (vector<DeclarationInstruction*>::iterator it = parameters->begin(); it != parameters->end(); it++)
		argTypes->push_back((*it)->type);
	return new TypePointerFunction(loc, resulttype, argTypes);
}
