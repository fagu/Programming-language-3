#include "parseresult.h"
#include "function.h"
#include "instruction.h"

int FunctionDeclaration::find() {
	int sizebefore = ParseRes->varstack.size();
	for (int i = 0; i < parameters->size(); i++) {
		(*parameters)[i]->find();
	}
	DeclarationInstruction * returndec = new DeclarationInstruction(Location(), resulttype, new string("return")); // TODO use real location
	returndec->find();
	instructions->find();
	while(ParseRes->varstack.size() > sizebefore) {
		ParseRes->vars.erase(ParseRes->vars.find(*ParseRes->varstack.top()->name));
		ParseRes->varstack.pop();
	}
	return returndec->pos;
}
