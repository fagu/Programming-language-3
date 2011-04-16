#ifndef TYPE_H
#define TYPE_H

#include <vector>
#include <string>
#include <map>
#include "location.h"
#include "opcodes.h"
#include "function.h"
#include "environment.h"
#include "instruction.h"

using namespace std;
class ArrayType;

class Type {
private:
	ArrayType * array;
public:
	Location loc;
	Type() {array = 0;}
	Type(Location _loc) : loc(_loc) {array = 0;}
	virtual ~Type() {}
	/**
	 * The size of this type on the stack
	 **/
	virtual int size()=0;
	enum STYLE {STYLE_CLASS, STYLE_PRIMITIVE, STYLE_NULL, STYLE_ARRAY, STYLE_FUNCTION};
	virtual STYLE style()=0;
	/**
	 * The distance of this type to the type t. The bigger the distance the more favored is the implicit conversion from this to t.
	 * 
	 * A distance of INFTY means that you cannot implicitly converted from this to t.
	 * 
	 * A distance of 0 means that the types are equal.
	 **/
	virtual int distance(Type *t);
	/**
	 * Creates an instruction which implicitly converts a from this type to the type t.
	 * 
	 * This is only safe if distance(t) != INFTY
	 * 
	 * find() still has to be called on the instruction
	 * @param a Expression to convert from.
	 * @param t Type to convert to.
	 * @return Conversion instruction
	 **/
	virtual Instruction * convertTo(Expression *a, Type *t);
	ArrayType * arrayType();
};

/**
 * TypePointer points to a type (e.g. by name).
 * 
 * It is needed when a type can be read from the source but could still be unknown at that point in the source.
 **/
class TypePointer {
protected:
	Type * type;
public:
	Location loc;
	TypePointer(Location _loc) : type(0), loc(_loc) {}
	virtual ~TypePointer() {}
	/**
	 * Finds out what this is pointing to
	 **/
	virtual void find() = 0;
	/**
	 * Returns the type that is pointed to
	 **/
	Type & operator*();
	/**
	 * Returns the type that is pointed to
	 **/
	Type * operator->();
	/**
	 * Returns the type that is pointed to
	 **/
	Type * real();
};

/**
 * A type given by its name
 **/
class TypePointerId : public TypePointer {
private:
	string * name;
public:
	TypePointerId(Location _loc, string *_name) : TypePointer(_loc), name(_name) {type = 0;}
	TypePointerId(Location _loc, Type * _type) : TypePointer(_loc) {type=_type;}
	~TypePointerId();
	void find();
};

/**
 * An array
 **/
class TypePointerArray : public TypePointer {
private:
	TypePointer * contenttype;
public:
	TypePointerArray(Location _loc, TypePointer * _contenttype) : TypePointer(_loc), contenttype(_contenttype) {}
	~TypePointerArray();
	void find();
};

/**
 * A function
 **/
class TypePointerFunction : public TypePointer {
private:
	TypePointer * returnType;
	vector<TypePointer*> * argTypes;
public:
	TypePointerFunction(Location _loc, TypePointer * _returnType, vector<TypePointer*> * _argTypes) : TypePointer(_loc), returnType(_returnType), argTypes(_argTypes) {}
	~TypePointerFunction();
	void find();
};

/**
 * A type which is already known explicitly at the time of the construction of the type pointer
 **/
// TODO Remove this class. It should not be needed.
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
	Environment *env;
	vector<FunctionDeclaration*> funcs;
	ClassType();
	~ClassType();
	void addVariable(VariableDeclaration *dec);
	void addFunction(FunctionDeclaration* func);
	/**
	 * Visits the type to determine its heap size
	 **/
	void find();
	/**
	 * Add all functions to the environment of this class
	 **/
	void findFuncs();
	int size();
	int hashsize();
	STYLE style() {return STYLE_CLASS;}
	int distance(Type* t);
};

/**
 * A primitive type which is defined by the compiler (e.g. int, bool)
 **/
class PrimitiveType : public Type {
private:
	int m_size;
public:
	PrimitiveType(Location _loc, int _size) : Type(_loc), m_size(_size) {}
	~PrimitiveType() {}
	int size() {
		return m_size;
	}
	STYLE style() {return STYLE_PRIMITIVE;}
	int distance(Type* t);
};

/**
 * The type of the expression "null"
 **/
class NullType : public Type {
public:
	NullType(Location _loc) : Type(_loc) {}
	int size() {return 1;}
	STYLE style() {return STYLE_NULL;}
	int distance(Type* t);
};

class ArrayType : public Type {
	friend class Type;
public:
	Type * contenttype;
	int size() {return 1;}
	STYLE style() {return STYLE_ARRAY;}
private:
	ArrayType(Location _loc, Type * _contenttype) : Type(_loc), contenttype(_contenttype) {}
};

class FunctionType : public Type {
public:
	Type * returnType;
	vector<Type*> * argTypes;
	FunctionType(Location _loc, Type * _returnType, vector<Type*> * _argTypes) : Type(_loc), returnType(_returnType), argTypes(_argTypes) {}
	~FunctionType();
	int size() {return 1;}
	STYLE style() {return STYLE_FUNCTION;}
};

#endif
