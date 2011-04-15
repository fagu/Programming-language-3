#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <vector>
#include <map>
#include <stack>
#include <string>
using namespace std;

class Environment;
class Type;
class VariableAccessor;
class Function;

class Environment {
private:
	map<string,VariableAccessor*> vars;
	map<string,Type*> types;
	vector<Environment*> parents;
	
	vector<VariableAccessor*> addedvars;
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
	
	void addVariable(VariableAccessor *var);
	//void addType(const string &name, Type *type);
	void addFunction(Function *func);
	void addParent(Environment *env);
	
	void enterBlock();
	void exitBlock();
	
	enum MESSAGE {OK, NONEFOUND, MULTIPLEFOUND};
	VariableAccessor * findVariable(const string &name);
	//Type * findType(const string &name);
	Function * findFunction(string *name, const vector<Type*> &argtypes, MESSAGE &message);
};

#endif // ENVIRONMENT_H