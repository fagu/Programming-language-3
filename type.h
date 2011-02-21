#ifndef TYPE_H
#define TYPE_H

#include <vector>
#include <string>

class BlockInstruction;
using namespace std;

class Type {
public:
	virtual ~Type() {}
	virtual void find() {}
	virtual int size()=0;
};

class TypePointer {
private:
	string * name;
	Type * type;
public:
	TypePointer(string *_name);
	~TypePointer();
	void find();
	Type & operator*();
	Type * operator->();
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

typedef vector<VariableDeclaration*> VariableDeclarations;

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
};

class FunctionDeclaration {
private:
	BlockInstruction* instructions;
public:
	FunctionDeclaration(BlockInstruction* _instructions) : instructions(_instructions) {}
	~FunctionDeclaration() {}
};

#endif
