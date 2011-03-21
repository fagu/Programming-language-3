#ifndef TYPE_H
#define TYPE_H

#include <vector>
#include <string>
#include <map>
#include "location.h"
#include "opcodes.h"

class Instruction;
class BlockInstruction;
class DeclarationInstruction;
using namespace std;

class ArrayType;

class Type {
private:
	ArrayType * array;
public:
	Location loc;
	Type(Location _loc) : loc(_loc) {array = 0;}
	virtual ~Type() {}
	virtual void find() {}
	virtual int size()=0;
	virtual char style()=0;
	virtual int distance(Type *t) ;
	virtual Instruction * convertTo(Instruction *a, Type *t);
	ArrayType * arrayType();
};

class TypePointer {
protected:
	Type * type;
public:
	Location loc;
	TypePointer(Location _loc) : type(0), loc(_loc) {}
	virtual ~TypePointer() {}
	virtual void find() = 0;
	Type & operator*();
	Type * operator->();
	Type * real();
};

class TypePointerId : public TypePointer {
private:
	string * name;
public:
	TypePointerId(Location _loc, string *_name) : TypePointer(_loc), name(_name) {type = 0;}
	TypePointerId(Location _loc, Type * _type) : TypePointer(_loc) {type=_type;}
	~TypePointerId();
	void find();
};

class TypePointerArray : public TypePointer {
private:
	TypePointer * contenttype;
public:
	TypePointerArray(Location _loc, TypePointer * _contenttype) : TypePointer(_loc), contenttype(_contenttype) {}
	~TypePointerArray();
	void find();
	
};

class TypePointerExplicit : public TypePointer {
public:
	TypePointerExplicit(Type *_type) : TypePointer(Location()) {type = _type;}
	~TypePointerExplicit() {}
	void find() {}
};

class VariableDeclaration {
public:
	Location loc;
	string *name;
	TypePointer *type;
	int pos;
public:
	VariableDeclaration(Location _loc, string *_name, TypePointer *_type) : loc(_loc), name(_name), type(_type), pos(-1) {
	}
	~VariableDeclaration() {
		delete name;
		delete type;
	}
};

typedef map<string,VariableDeclaration*> VariableDeclarations;

class ClassType : public Type {
private:
	VariableDeclarations *declarations;
	int m_size;
public:
	string *name;
	ClassType(Location _loc, string *_name, VariableDeclarations *_declarations) : Type(_loc), name(_name), declarations(_declarations), m_size(-1) {}
	~ClassType();
	void find();
	int size();
	int hashsize();
	VariableDeclaration *var(const string &name);
	char style() {return 'C';}
	int distance(Type* t);
};

class PrimitiveType : public Type {
private:
	int m_size;
public:
	PrimitiveType(Location _loc, int _size) : Type(_loc), m_size(_size) {}
	~PrimitiveType() {}
	int size() {
		return m_size;
	}
	char style() {return 'P';}
	int distance(Type* t);
};

class NullType : public Type {
public:
	NullType(Location _loc) : Type(_loc) {}
	int size() {return 1;}
	char style() {return 'N';}
	int distance(Type* t) {return true;}
};

class ArrayType : public Type {
	friend class Type;
public:
	Type * contenttype;
	int size() {return 1;}
	char style() {return 'A';}
private:
	ArrayType(Location _loc, Type * _contenttype) : Type(_loc), contenttype(_contenttype) {}
};

class Function {
public:
	string *name;
	vector<DeclarationInstruction*> * parameters;
	TypePointer * resulttype;
	Function(string *_name, vector<DeclarationInstruction*> * _parameters, TypePointer * _resulttype) : name(_name), parameters(_parameters), resulttype(_resulttype) {}
	virtual ~Function() {}
	virtual char type() = 0;
};

class FunctionDeclaration : public Function {
private:
	BlockInstruction* instructions;
public:
	Location loc;
	int num;
	FunctionDeclaration(Location _loc, string *_name, vector<DeclarationInstruction*> * _parameters, BlockInstruction * _instructions, TypePointer * _resulttype) : Function(_name, _parameters, _resulttype), loc(_loc), instructions(_instructions) {}
	~FunctionDeclaration() {}
	char type() {return 'D';}
	int find();
};

class PrimitiveFunction : public Function {
public:
	OPCODE op;
	PrimitiveFunction(string *_name, vector<DeclarationInstruction*> * _parameters, OPCODE _op, TypePointer * _resulttype) : Function(_name, _parameters, _resulttype), op(_op) {}
	~PrimitiveFunction() {}
	char type() {return 'V';}
};

#endif
