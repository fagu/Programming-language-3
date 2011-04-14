#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <vector>
#include <map>
#include <stack>
#include <string>
using namespace std;

class Environment;
class Type;
class DeclarationInstruction;
class Function;

class Environment {
private:
	map<string,DeclarationInstruction*> vars;
	map<string,Type*> types;
	vector<Environment*> parents;
	
	vector<DeclarationInstruction*> addedvars;
	vector<Function*> addedfunctions;
	vector<pair<string,Type*> > addedtypes;
	stack<int> varnum;
	stack<int> typenum;
	stack<int> funcnum;
	stack<int> parnum;
public:
	multimap<string,Function*> functions;
	Environment();
	Environment(Environment *par);
	
	void addVariable(DeclarationInstruction *var);
	//void addType(const string &name, Type *type);
	void addFunction(Function *func);
	
	void enterBlock();
	void exitBlock();
	
	enum MESSAGE {OK, NONEFOUND, MULTIPLEFOUND};
	DeclarationInstruction * findVariable(const string &name);
	//Type * findType(const string &name);
	Function * findFunction(string *name, const vector<Type*> &argtypes, MESSAGE &message);
};

#endif // ENVIRONMENT_H