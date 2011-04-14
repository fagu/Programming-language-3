#include <stdio.h>
#include "type.h"
#include "parseresult.h"

int Type::distance(Type* t) {
	if (t == this)
		return 0;
	else
		return INFTY;
}

Instruction* Type::convertTo(Instruction* a, Type* t) {
	return a;
}

ArrayType* Type::arrayType() {
	if (!array)
		array = new ArrayType(Location(), this);
	return array;
}

ClassType::ClassType() : Type(), m_size(-1) {
	declarations = new VariableDeclarations();
	env = new Environment;
}

ClassType::~ClassType() {
	for (VariableDeclarations::iterator it = declarations->begin(); it != declarations->end(); it++) {
		delete it->second;
	}
	delete declarations;
	delete env;
}

void ClassType::addVariable(VariableDeclaration* dec) {
	if ((*declarations)[*dec->name])
		fprintf(stderr, "Multiple declaration of '%s'!\n", dec->name->c_str());
	(*declarations)[*dec->name] = dec;
}

void ClassType::addFunction(Function* func) {
	env->addFunction(func);
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

int ClassType::distance(Type* t) {
	if (t == this)
		return 0;
	else if (t == ParseRes->boolType)
		return INFTY/2;
	else
		return INFTY;
}

int PrimitiveType::distance(Type* t) {
	if (this == t)
		return 0;
	if (this == ParseRes->intType) {
		if (t == ParseRes->boolType || t == ParseRes->charType)
			return 1;
		else
			return INFTY;
	} else if (this == ParseRes->charType) {
		if (t == ParseRes->boolType || t == ParseRes->intType)
			return 1;
		else
			return INFTY;
	} else
		return INFTY;
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
