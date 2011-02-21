#include <stdio.h>
#include "type.h"
#include "parseresult.h"

TypePointer::TypePointer(string* _name) : name(_name), type(0) {
}

TypePointer::~TypePointer() {
	delete name;
}

void TypePointer::find() {
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

ClassType::ClassType(VariableDeclarations* _declarations) : declarations(_declarations), m_size(-1), infind(false) {
}

ClassType::~ClassType() {
	for (VariableDeclarations::iterator it = declarations->begin(); it != declarations->end(); it++) {
		delete *it;
	}
}

void ClassType::find() {
	if (infind) {
		fprintf(stderr, "Cycle in type definitions!\n");
		return;
	}
	infind = true;
	int size = 0;
	for (VariableDeclarations::iterator it = declarations->begin(); it != declarations->end(); it++) {
		(*it)->pos = size;
		size += (*(*it)->type)->size();
	}
	m_size = size;
	infind = false;
}

int ClassType::size() {
	if (m_size == -1)
		find();
	return m_size;
}
