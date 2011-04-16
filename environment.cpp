#include "environment.h"
#include "instruction.h"
#include "function.h"
#include "type.h"

Environment::Environment() {
}

Environment::Environment(Environment* par) {
	parents.push_back(par);
}

void Environment::addVariable(VariableAccessor* var) {
	if (vars.count(*var->name))
		fprintf(stderr, "Variable '%s' already exists!\n", var->name->c_str());
	vars[*var->name] = var;
	addedvars.push_back(var);
}

/*void Environment::addType(const string &name, Type* type) {
	if (types.count(name))
		fprintf(stderr, "Type '%s' already exists!\n", name.c_str());
	types[name] = type;
	addedtypes.push_back(make_pair(name,type));
}*/

void Environment::addFunction(FunctionAccessor* func) {
	functions.insert(make_pair(*func->name, func));
	addedfunctions.push_back(func);
}

void Environment::addParent(Environment* env) {
	parents.push_back(env);
}

void Environment::enterBlock() {
	varnum.push(addedvars.size());
	//typenum.push(addedtypes.size());
	funcnum.push(addedfunctions.size());
	parnum.push(parents.size());
}

void Environment::exitBlock() {
	while(addedvars.size() > varnum.top()) {
		vars.erase(vars.find(*addedvars.back()->name));
		addedvars.pop_back();
	}
	varnum.pop();
	/*while(addedtypes.size() > typenum.top()) {
		types.erase(types.find(addedtypes.back().first));
		addedtypes.pop_back();
	}
	typenum.pop();*/
	while(addedfunctions.size() > funcnum.top()) {
		FunctionAccessor *func = addedfunctions.back();
		pair<multimap<string,FunctionAccessor*>::iterator,multimap<string,FunctionAccessor*>::iterator> pa = functions.equal_range(*func->name);
		for (multimap<string,FunctionAccessor*>::iterator it = pa.first; it != pa.second; it++) {
			if (it->second == func) {
				functions.erase(it);
				break;
			}
		}
		addedfunctions.pop_back();
	}
	funcnum.pop();
	while(parents.size() > parnum.top())
		parents.pop_back();
	parnum.pop();
}

VariableAccessor * Environment::findVariable(const std::string& name) {
	if (vars.count(name))
		return vars[name];
	for (int i = parents.size()-1; i >= 0; i--) {
		VariableAccessor * r = parents[i]->findVariable(name);
		if (r)
			return r;
	}
	return 0;
}

/*Type * Environment::findType(const std::string& name) {
	if (types.count(name))
		return types[name];
	for (int i = parents.size()-1; i >= 0; i--) {
		Type * r = parents[i]->findType(name);
		if (r)
			return r;
	}
	return 0;
}*/

int Environment::addToDists(vector< Environment::Dist >& dists, string* name, const std::vector< Type* >& argtypes, int depth) {
	pair<multimap<string,FunctionAccessor*>::iterator,multimap<string,FunctionAccessor*>::iterator> pa = functions.equal_range(*name);
	for (multimap<string,FunctionAccessor*>::iterator it = pa.first; it != pa.second; it++) {
		FunctionAccessor * func = it->second;
		if (func->argtypes->size() != argtypes.size())
			continue;
		dists.push_back(Dist(func, vector<int>()));
		vector<int> & dv = dists.back().second;
		dv.push_back(depth);
		bool ok = true;
		for (int i = 0; i < argtypes.size(); i++) {
			dv.push_back(argtypes[i]->distance((*func->argtypes)[i]));
			if (dv.back() == INFTY) {
				ok = false;
				break;
			}
		}
		if (!ok)
			dists.pop_back();
	}
	depth++;
	for (int i = parents.size()-1; i >= 0; i--) {
		depth = parents[i]->addToDists(dists, name, argtypes, depth);
	}
	return depth;
}

FunctionAccessor* Environment::findFunction(string* name, const std::vector< Type* >& argtypes, Environment::MESSAGE& message) {
	vector<Dist> dists;
	addToDists(dists, name, argtypes, 0);
	if (dists.empty()) {
		for (int i = parents.size()-1; i >= 0; i--) {
			FunctionAccessor * r = parents[i]->findFunction(name, argtypes, message);
			if (message != NONEFOUND)
				return r;
		}
		message = NONEFOUND;
		return 0;
	}
	vector<int> smallest(argtypes.size()+1, INFTY);
	for (int i = 0; i < dists.size(); i++) {
		for (int k = 0; k < argtypes.size()+1; k++) {
			smallest[k] = min(smallest[k], dists[i].second[k]);
		}
	}
	FunctionAccessor *func = 0;
	for (int i = 0; i < dists.size(); i++) {
		bool ok = true;
		for (int k = 0; k < argtypes.size()+1; k++) {
			if (smallest[k] != dists[i].second[k]) {
				ok = false;
				break;
			}
		}
		if (ok) {
			if (func) {
				message = MULTIPLEFOUND;
				return 0;
			} else
				func = dists[i].first;
		}
	}
	if (!func) {
		message = MULTIPLEFOUND;
		return 0;
	}
	message = OK;
	return func;
}
