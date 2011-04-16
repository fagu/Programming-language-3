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
class FunctionAccessor;

/**
 * The Environment class manages variables and functions and their scopes.
 * 
 * Environments can have parent environments.
 * For example the environment inside a function has the global scope environment (and possibly the surrounding class environment) as a parent.
 * 
 * Below the environment is considered to not include its parents.
 * 
 * @section VARS Variables
 * 
 * Inside an environment no two variables of the same name may be declared.
 * 
 * To find a variable the name is first looked up in this environment. If it does not exist it will be looked up in the parent environments in reverse order (the earlier a parents has been inserted the later it will be scanned) with the same procedure.
 * 
 * @section FUNCS Functions
 * 
 * Multiple functions with the same name may be declared. Even multiple functions with the same name and the same argument types may be declared but this will give an ambiguity error when they should be called.
 * 
 * When a function is to be found all functions with the same name and expected argument types to which the arguments can be implicitly converted are retrieved (from this environment, its parents, its grandparents, ...).
 * 
 * Each such function is assigned a vector. The first coordinate in this vector is the depth of the environment this function was found in. The depth is the discovery time of the environment during a depth-first search in which parents are visited in reverse order (the earlier a parents has been inserted the later it will be visited). The other coordinates are the implicit conversion distances of the arguments to their expected types.
 * 
 * If there is a function such that its vector is strictly dominated by all other vectors (all coordinates of this vector are the smallest), then it will be returned. Otherwise an ambiguity has been detected (or no function has been found at all).
 **/
// TODO Produce an error whenever multiple functions with the same name and argument types are defined
class Environment {
private:
	map<string,VariableAccessor*> vars;
	multimap<string,FunctionAccessor*> functions;
	//map<string,Type*> types;
	vector<Environment*> parents;
	
	vector<VariableAccessor*> addedvars;
	vector<FunctionAccessor*> addedfunctions;
	//vector<pair<string,Type*> > addedtypes;
	stack<int> varnum;
	//stack<int> typenum;
	stack<int> funcnum;
	stack<int> parnum;
	
	typedef pair<FunctionAccessor*,vector<int> > Dist;
	int addToDists(vector<Dist> &dists, string* name, const std::vector< Type* >& argtypes, int depth);
public:
	Environment();
	Environment(Environment *par);
	
	void addVariable(VariableAccessor *var);
	//void addType(const string &name, Type *type);
	void addFunction(FunctionAccessor *func);
	void addParent(Environment *env);
	
	void enterBlock();
	void exitBlock();
	
	enum MESSAGE {OK, NONEFOUND, MULTIPLEFOUND};
	VariableAccessor * findVariable(const string &name);
	//Type * findType(const string &name);
	// TODO This has bad asymptotic behaviour O(N) when there are N functions with the same name
	FunctionAccessor* findFunction(string* name, const std::vector< Type* >& argtypes, Environment::MESSAGE& message);
};

#endif // ENVIRONMENT_H