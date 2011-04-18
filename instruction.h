#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdio.h>
#include <vector>
using namespace std;
#include "location.h"
#include "opcodes.h"
#include "environment.h"

class Type;
class TypePointer;
class ClassType;
class VariableDeclaration;
class BlockInstruction;
class FunctionDefinition;

/**
 * Expression is the base class for everything that returns a value (possibly void) and can in some cases be assigned values
 **/
class Expression {
public:
	/**
	 * The stack reference position of the return value
	 **/
	int pos;
	virtual ~Expression() {}
	/**
	 * The type of the return value
	 **/
	virtual Type *resulttype() = 0;
};

/**
 * The base class for all expressions which can be called directly
 **/
class Instruction : public Expression {
public:
	Location loc;
	Instruction(Location _loc) : loc(_loc) {}
	virtual ~Instruction() {}
	/**
	 * Tries to calculate the value
	 * 
	 * Checks correctness (if functions exists, types match, ...) and writes opcodes to ParseRes afterwards
	 **/
	virtual void find(Environment *e) = 0;
	/**
	 * Tries to assign the return value of b
	 * 
	 * Checks correctness (if functions exists, types match, ...) and writes opcodes to ParseRes afterwards
	 **/
	virtual void findSet(Environment *e, Instruction *b) {fprintf(stderr, "This expression cannot be assigned a value!\n");};
};

class IntegerConstantInstruction : public Instruction {
private:
	int co;
public:
	IntegerConstantInstruction(Location _loc, int _co) : Instruction(_loc), co(_co) {}
	void find(Environment *e);
	Type* resulttype();
};

class CharacterConstantInstruction : public Instruction {
private:
	char co;
public:
	CharacterConstantInstruction(Location _loc, char _co) : Instruction(_loc), co(_co) {}
	void find(Environment *e);
	Type* resulttype();
};

class StringConstantInstruction : public Instruction {
private:
	string *co;
public:
	StringConstantInstruction(Location _loc, string *_co) : Instruction(_loc), co(_co) {}
	void find(Environment *e);
	Type* resulttype();
};

class NullInstruction : public Instruction {
public:
	NullInstruction(Location _loc) : Instruction(_loc) {}
	void find(Environment *e);
	Type* resulttype();
};

class NewInstruction : public Instruction {
private:
	string *name;
	ClassType *type;
public:
	NewInstruction(Location _loc, string *_name) : Instruction(_loc), name(_name) {}
	void find(Environment *e);
	Type* resulttype();
};

/**
 * Instruction to declare a variable
 **/
class DeclarationInstruction : public Instruction {
public:
	TypePointer *type;
	string *name;
	int varpos;
public:
	DeclarationInstruction(Location _loc, TypePointer *_type, string *_name) : Instruction(_loc), type(_type), name(_name) {}
	void find(Environment *e);
	Type* resulttype();
	int getPos(Environment* e, Instruction* par);
};

/**
 * Instruction to assign the value of b to a (calls findSet(e,b) on a)
 **/
class SetInstruction : public Instruction {
private:
	Instruction *a;
	Instruction *b;
public:
	SetInstruction(Location _loc, Instruction *_a, Instruction *_b) : Instruction(_loc), a(_a), b(_b) {}
	void find(Environment *e);
	Type* resulttype();
};

/**
 * The value of a variable without explicitly given object
 **/
class VariableInstruction : public Instruction {
private:
	string *name;
	VariableAccessor *acc;
public:
	VariableInstruction(Location _loc, string *_name) : Instruction(_loc), name(_name) {}
	void find(Environment *e);
	Type* resulttype();
	void findSet(Environment *e, Instruction* b);
};

class ExplicitVariableInstruction : public Instruction {
private:
	VariableAccessor *acc;
public:
	ExplicitVariableInstruction(Location _loc, VariableAccessor *_acc) : Instruction(_loc), acc(_acc) {}
	void find(Environment *e);
	Type* resulttype();
	void findSet(Environment *e, Instruction* b);
};

/**
 * The value of a member variable
 **/
class AccessInstruction : public Instruction {
private:
	string *name;
	VariableAccessor *acc;
	Instruction *a;
public:
	AccessInstruction(Location _loc, Instruction *_a, string *_name) : Instruction(_loc), a(_a), name(_name) {}
	void find(Environment *e);
	Type* resulttype();
	void findSet(Environment *e, Instruction* b);
};

/**
 * The value of an array entry
 **/
class AccessArrayInstruction : public Instruction {
private:
	Instruction * a;
	Instruction * b;
public:
	AccessArrayInstruction(Location _loc, Instruction * _a, Instruction * _b) : Instruction(_loc), a(_a), b(_b) {}
	void find(Environment *e);
	Type* resulttype();
	void findSet(Environment *e, Instruction* c);
};

class IfInstruction : public Instruction {
private:
	Instruction *cond;
	BlockInstruction *then;
	BlockInstruction *Else;
public:
	IfInstruction(Location _loc, Instruction *_cond, BlockInstruction *_then, BlockInstruction *_else) : Instruction(_loc), cond(_cond), then(_then), Else(_else) {}
	void find(Environment *e);
	Type* resulttype();
};

class WhileInstruction : public Instruction {
private:
	Instruction *cond;
	BlockInstruction *then;
public:
	WhileInstruction(Location _loc, Instruction *_cond, BlockInstruction *_then) : Instruction(_loc), cond(_cond), then(_then) {}
	void find(Environment *e);
	Type* resulttype();
};

/**
 * The return value of a function call without explicitly given object
 **/
class CallInstruction : public Instruction {
private:
	string *name;
	FunctionAccessor *acc;
	vector<Instruction*> *arguments;
public:
	CallInstruction(Location _loc, string *_name, vector<Instruction*> *_arguments) : Instruction(_loc), name(_name), arguments(_arguments) {}
	void find(Environment *e);
	Type* resulttype();
};

/**
 * The return value of a member function call
 **/
class ClassCallInstruction : public Instruction {
private:
	Instruction *a;
	string *name;
	FunctionAccessor *acc;
	vector<Instruction*> *arguments;
public:
	ClassCallInstruction(Location _loc, Instruction *_a, string *_name, vector<Instruction*> *_arguments) : Instruction(_loc), a(_a), name(_name), arguments(_arguments) {}
	void find(Environment *e);
	Type* resulttype();
};

class CreateArrayInstruction : public Instruction {
private:
	TypePointer * contenttype;
	Instruction * size;
public:
	CreateArrayInstruction(Location _loc, TypePointer * _contenttype, Instruction *_size) : Instruction(_loc), contenttype(_contenttype), size(_size) {}
	void find(Environment *e);
	Type* resulttype();
};

/*class CreateFunctionInstruction : public Instruction {
public:
	vector<DeclarationInstruction*> * parameters;
	TypePointer * resulttype;
	Function(string *_name, vector<DeclarationInstruction*> * _parameters, TypePointer * _resulttype) : name(_name), parameters(_parameters), resulttype(_resulttype) {}
};*/

/**
 * Multiple instructions in the same scope which is inside the surrounding scope, returns void
 **/
class BlockInstruction : public Instruction {
public:
	vector<Instruction*> instructions;
public:
	BlockInstruction(Location _loc) : Instruction(_loc) {}
	BlockInstruction(Location _loc, Instruction* _i) : Instruction(_loc) {instructions.push_back(_i);}
	void find(Environment *e);
	Type* resulttype();
};

/**
 * Multiple instructions in the outer scope, returns void
 **/
class CompoundInstruction : public Instruction {
private:
	vector<Instruction*> * instructions;
public:
	CompoundInstruction(Location _loc, vector<Instruction*> *_instructions) : Instruction(_loc), instructions(_instructions) {}
	void find(Environment *e);
	Type* resulttype();
};

/**
 * Does nothing, returns void
 **/
class EmptyInstruction : public Instruction {
public:
	EmptyInstruction(Location _loc) : Instruction(_loc) {}
	void find(Environment *e);
	Type* resulttype();
};

/**
 * Returns the unmodified input, useful for conversions that do nothing
 **/
class StupidConvert : public Instruction {
private:
	Type *type;
public:
	StupidConvert(Expression *a, Type *t) : Instruction(Location()), type(t) {pos=a->pos;}
	void find(Environment* e) {}
	Type* resulttype() {return type;}
};

class FunctionDefinitionInstruction : public Instruction {
private:
	FunctionDefinition *func;
public:
	FunctionDefinitionInstruction(Location _loc, FunctionDefinition *_func) : Instruction(_loc), func(_func) {}
	void find(Environment* e);
	Type* resulttype();
};



/**
 * Base class to access the value of a variable
 **/
class VariableAccessor : public Expression {
protected:
public:
	string *name;
	VariableAccessor(string *_name) : name(_name) {}
	virtual ~VariableAccessor() {}
	/**
	 * Tries to calculate the value
	 * 
	 * Writes opcodes to ParseRes
	 * @param par The "parent" object (0 if the variable is accessed directly)
	 **/
	virtual void find(Environment *e, Expression *par) = 0;
	/**
	 * Tries to assign the return value of s
	 * 
	 * Writes opcodes to ParseRes
	 * @param par The "parent" object (0 if the variable is accessed directly)
	 **/
	virtual void findSet(Environment *e, Expression *par, Expression *s) = 0;
	Type *resulttype() = 0;
};

/**
 * Stack (aka local) variables
 **/
class VariableAccessorStack : public VariableAccessor {
private:
	Type *type;
public:
	VariableAccessorStack(string *_name, Type *_type, int _pos) : VariableAccessor(_name), type(_type) {pos=_pos;}
	void find(Environment* e, Expression* par) {}
	void findSet(Environment* e, Expression* par, Expression *s);
	Type *resulttype() {return type;}
};

/**
 * Heap (aka member) variables
 **/
class VariableAccessorHeap : public VariableAccessor {
private:
	Type *type;
	int posin;
public:
	VariableAccessorHeap(string *_name, Type *_type, int _posin) : VariableAccessor(_name), type(_type), posin(_posin) {}
	void find(Environment* e, Expression* par);
	void findSet(Environment* e, Expression* par, Expression *s);
	Type *resulttype() {return type;}
};

/**
 * Static (aka global) variables
 **/
class VariableAccessorStatic : public VariableAccessor {
private:
	VariableDeclaration *dec;
	//int globpos;
public:
	VariableAccessorStatic(string *_name, VariableDeclaration *_dec/*int _globpos*/) : VariableAccessor(_name), dec(_dec)/*globpos(_globpos)*/ {}
	void find(Environment* e, Expression* par);
	void findSet(Environment* e, Expression* par, Expression* s);
	Type *resulttype();
};



/**
 * Base class to run a function and access its return value
 **/
class FunctionAccessor : public Expression {
protected:
	Type *type;
public:
	string *name;
	vector<Type*> *argtypes;
	/**
	 * @param _argtypes The types of the arguments (excluding the implicit 'this' argument)
	 **/
	FunctionAccessor(string *_name, Type *_type, vector<Type*> *_argtypes) : name(_name), type(_type), argtypes(_argtypes) {}
	virtual ~FunctionAccessor() {}
	/**
	 * Tries to call the function
	 * 
	 * Writes opcodes to ParseRes
	 * @param par The "parent" object (0 if the function is accessed directly), has to be converted to the expected type
	 * @param args The arguments still have to be converted to the expected types
	 **/
	virtual void find(Environment *e, Expression *par, const vector<Expression*> &args) = 0;
	Type *resulttype() {return type;}
};

/**
 * Normal function
 **/
class FunctionAccessorNormal : public FunctionAccessor {
private:
	int funcnum;
	Type *parType;
public:
	FunctionAccessorNormal(string *_name, Type *_type, int _funcnum, vector<Type*> *_argtypes, Type *_parType) : FunctionAccessor(_name, _type, _argtypes), funcnum(_funcnum), parType(_parType) {}
	void find(Environment* e, Expression* par, const std::vector< Expression* >& args);
};

/**
 * Primitive function that has its own opcode (e.g. operator+)
 **/
class FunctionAccessorPrimitive : public FunctionAccessor {
private:
	OPCODE op;
public:
	FunctionAccessorPrimitive(string *_name, Type *_type, OPCODE _op, vector<Type*> *_argtypes) : FunctionAccessor(_name, _type, _argtypes), op(_op) {}
	void find(Environment* e, Expression* par, const std::vector< Expression* >& args);
};

class FunctionAccessorPointer : public FunctionAccessor {
private:
	VariableAccessor *fp;
	Type *parType;
public:
	FunctionAccessorPointer(string* _name, Type* _type, VariableAccessor *_fp, std::vector< Type* >* _argtypes, Type *_parType) : FunctionAccessor(_name, _type, _argtypes), fp(_fp), parType(_parType) {}
	void find(Environment* e, Expression* par, const std::vector< Expression* >& args);
};

#endif
