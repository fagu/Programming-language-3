#ifndef TYPE_H
#define TYPE_H

#include <vector>
#include <string>
#include <map>

class BlockInstruction;
class DeclarationInstruction;
using namespace std;

class Type {
public:
	virtual ~Type() {}
	virtual void find() {}
	virtual int size()=0;
	virtual char style()=0;
	virtual bool convertibleTo(Type *t) {return t == this;};
};

class TypePointer {
private:
	string * name;
	Type * type;
public:
	TypePointer(string *_name) : name(_name), type(0) {}
	TypePointer(Type * _type) : type(_type) {}
	~TypePointer();
	void find();
	Type & operator*();
	Type * operator->();
	Type * real();
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
