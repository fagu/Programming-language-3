#include <stdio.h>
#include "type.h"
#include "parseresult.h"

Instruction* Type::convertTo(Instruction* a, Type* t) {
	return a;
}

ArrayType* Type::arrayType() {
	if (!array)
		array = new ArrayType(Location(), this);
	return array;
}

ClassType::~ClassType() {
	for (VariableDeclarations::iterator it = declarations->begin(); it != declarations->end(); it++) {
		delete it->second;
	}
	delete declarations;
}

void ClassType::find() {
	int size = 0;
	for (VariableDeclarations::iterator it = declarations->begin(); it != declarations->end(); it++) {
		it->second->pos = size;
		size += (*it->second->type)->size();
	}
	m_size = size;
}

int ClassType::size() {
	return 1;
}

int ClassType::hashsize() {
	if (m_size == -1)
		find();
	return m_size;
}

bool ClassType::convertibleTo(Type* t) {
	return t == this || t == ParseRes->boolType;
}

VariableDeclaration* ClassType::var(const std::string& name) {
	return (*declarations)[name];
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

TypePointerId::~TypePointerId() {
	delete name;
}

void TypePointerId::find() {
	if (type)
		return;
	type = ParseRes->types[*name];
	if (type == 0)
		printerr("Type '%s' not known!\n", name->c_str());
}

TypePointerArray::~TypePointerArray() {
	delete contenttype;
}

void TypePointerArray::find() {
	contenttype->find();
	type = contenttype->real()->arrayType();
}

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
