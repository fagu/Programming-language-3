#include <stdio.h>
#include "type.h"
#include "parseresult.h"

TypePointer::~TypePointer() {
	delete name;
}

void TypePointer::find() {
	if (type)
		return;
	type = ParseRes->types[*name];
	if (type == 0)
		fprintf(stderr, "Type '%s' not known!\n", name->c_str());
}

Type& TypePointer::operator*() {
	if (type == 0)
		find();
	return *type;
}

Type* TypePointer::operator->() {
	if (type == 0)
		find();
	return type;
}

Type* TypePointer::real() {
	if (type == 0)
		find();
	return type;
}

ClassType::ClassType(VariableDeclarations* _declarations) : declarations(_declarations), m_size(-1), infind(false) {
}

ClassType::~ClassType() {
	for (VariableDeclarations::iterator it = declarations->begin(); it != declarations->end(); it++) {
		delete it->second;
	}
	delete declarations;
}

void ClassType::find() {
	if (infind) {
		fprintf(stderr, "Cycle in type definitions!\n");
		return;
	}
	infind = true;
	int size = 0;
	for (VariableDeclarations::iterator it = declarations->begin(); it != declarations->end(); it++) {
		it->second->pos = size;
		size += (*it->second->type)->size();
	}
	m_size = size;
	infind = false;
}

int ClassType::size() {
	return 1;
}

int ClassType::hashsize() {
	if (m_size == -1)
		find();
	return m_size;
}

VariableDeclaration* ClassType::var(const std::string& name) {
	return (*declarations)[name];
}

int FunctionDeclaration::find() {
	int sizebefore = ParseRes->varstack.size();
	for (int i = 0; i < parameters->size(); i++) {
		(*parameters)[i]->find();
	}
	DeclarationInstruction * returndec = new DeclarationInstruction(resulttype, new string("return"));
	returndec->find();
	instructions->find();
	while(ParseRes->varstack.size() > sizebefore) {
		ParseRes->vars.erase(ParseRes->vars.find(*ParseRes->varstack.top()->name));
		ParseRes->varstack.pop();
	}
	return returndec->pos;
}
