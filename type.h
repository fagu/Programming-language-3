#ifndef TYPE_H
#define TYPE_H

#include <vector>
#include <string>
#include <map>
#include "location.h"

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
	virtual bool convertibleTo(Type *t) {return t == this;};
	virtual Instruction * convertTo(Instruction *a, Type *t);
	ArrayType * arrayType();
};

class TypePointer {
protected:
	Type * type;
public:
	Location loc;
	TypePointer(Location _loc) : loc(_loc) {}
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
	bool convertibleTo(Type* t);
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
};

class NullType : public Type {
public:
	NullType(Location _loc) : Type(_loc) {}
	int size() {return 1;}
	char style() {return 'N';}
	bool convertibleTo(Type* t) {return true;}
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

class FunctionDeclaration {
private:
	BlockInstruction* instructions;
public:
	TypePointer * resulttype;
	vector<DeclarationInstruction*> * parameters;
	int num;
	FunctionDeclaration(vector<DeclarationInstruction*> * _parameters, BlockInstruction * _instructions, TypePointer * _resulttype) : parameters(_parameters), instructions(_instructions), resulttype(_resulttype) {}
	~FunctionDeclaration() {}
	int find();
};

#endif
