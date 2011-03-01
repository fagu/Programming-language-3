#ifndef TYPE_H
#define TYPE_H

#include <vector>
#include <string>
#include <map>

class BlockInstruction;
class DeclarationInstruction;
using namespace std;

class ArrayType;

class Type {
private:
	ArrayType * array;
public:
	Type() {array = 0;}
	virtual ~Type() {}
	virtual void find() {}
	virtual int size()=0;
	virtual char style()=0;
	virtual bool convertibleTo(Type *t) {return t == this;};
	ArrayType * arrayType();
};

class TypePointer {
protected:
	Type * type;
public:
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
	TypePointerId(string *_name) : name(_name) {type = 0;}
	TypePointerId(Type * _type) {type=_type;}
	~TypePointerId();
	void find();
};

class TypePointerArray : public TypePointer {
private:
	TypePointer * contenttype;
public:
	TypePointerArray(TypePointer * _contenttype) : contenttype(_contenttype) {}
	~TypePointerArray();
	void find();
	
};

class VariableDeclaration {
public:
	string *name;
	TypePointer *type;
	int pos;
public:
	VariableDeclaration(string *_name, TypePointer *_type) : name(_name), type(_type), pos(-1) {
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
	bool infind;
public:
	ClassType(VariableDeclarations *_declarations);
	~ClassType();
	void find();
	int size();
	int hashsize();
	VariableDeclaration *var(const string &name);
	char style() {return 'C';}
};

class PrimitiveType : public Type {
private:
	int m_size;
public:
	PrimitiveType(int _size) : m_size(_size) {}
	~PrimitiveType() {}
	int size() {
		return m_size;
	}
	char style() {return 'P';}
};

class NullType : public Type {
public:
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
	ArrayType(Type * _contenttype) : contenttype(_contenttype) {}
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
