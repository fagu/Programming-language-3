#include <stdio.h>
#include "type.h"
#include "parseresult.h"

int Type::distance(Type* t) {
	if (t == this)
		return 0;
	else
		return INFTY;
}

Instruction* Type::convertTo(Expression* a, Type* t) {
	return new StupidConvert(a, t);
}

ArrayType* Type::arrayType() {
	if (!array)
		array = new ArrayType(Location(), this);
	return array;
}

FunctionType::~FunctionType() {
	delete argTypes;
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

void ClassType::addFunction(FunctionDefinition* func) {
	funcs.push_back(func);
}

void ClassType::find() {
	int size = 0;
	for (VariableDeclarations::iterator it = declarations->begin(); it != declarations->end(); it++) {
		it->second->pos = size;
		env->addVariable(new VariableAccessorHeap(it->second->name, it->second->type->real(), it->second->pos));
		size += (*it->second->type)->size();
	}
	m_size = size;
}

void ClassType::findFuncs() {
	for (vector<FunctionDefinition*>::iterator it = funcs.begin(); it != funcs.end(); it++) {
		FunctionDefinition *func = *it;
		vector<Type*> *argTypes = new vector<Type*>;
		for (int i = 1; i < func->parameters->size(); i++)
			argTypes->push_back((*func->parameters)[i]->type->real());
		FunctionAccessorNormal *fa = new FunctionAccessorNormal(func->name, func->resulttype->real(), func->num, argTypes, (*func->parameters)[0]->type->real());
		env->addFunction(fa);
	}
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

int NullType::distance(Type* t) {
	if (t->style() == Type::STYLE_CLASS)
		return 1;
	else
		return INFTY;
}

int FunctionType::distance(Type* t) {
	if (t->style() != Type::STYLE_FUNCTION)
		return INFTY;
	FunctionType *ft = dynamic_cast<FunctionType*>(t);
	if (returnType->distance(ft->returnType) != 0)
		return INFTY;
	if (argTypes->size() != ft->argTypes->size())
		return INFTY;
	if ((parType == 0) != (ft->parType == 0))
		return INFTY;
	if (parType && parType->distance(ft->parType) != 0)
		return INFTY;
	for (int i = 0; i < argTypes->size(); i++) {
		if ((*argTypes)[i]->distance((*ft->argTypes)[i]) != 0)
			return INFTY;
	}
	return 0;
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

TypePointerFunction::~TypePointerFunction() {
	delete returnType;
	for (int i = 0; i < (int)argTypes->size(); i++)
		delete (*argTypes)[i];
	delete argTypes;
}

void TypePointerFunction::find() {
	vector<Type*> *args = new vector<Type*>(argTypes->size());
	for (int i = 0; i < (int)argTypes->size(); i++)
		(*args)[i] = (*argTypes)[i]->real();
	type = new FunctionType(loc, returnType->real(), args);
}
