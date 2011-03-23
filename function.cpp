#include "parseresult.h"
#include "function.h"
#include "instruction.h"
#include "type.h"

int FunctionDeclaration::find() {
	int sizebefore = ParseRes->varstack.size();
	for (int i = 0; i < parameters->size(); i++) {
		(*parameters)[i]->find();
	}
	DeclarationInstruction * returndec = new DeclarationInstruction(Location(), resulttype, new string("return")); // TODO use real location
	returndec->find();
	instructions->find();
	while(ParseRes->varstack.size() > sizebefore) {
		ParseRes->vars.erase(ParseRes->vars.find(*ParseRes->varstack.top()->name));
		ParseRes->varstack.pop();
	}
	return returndec->pos;
}

void FunctionSet::addFunction(Function* func) {
	functions.insert(make_pair(*func->name, func));
}

typedef pair<Function*,vector<int> > Dist;

Function* FunctionSet::findFunction(string* name, const vector< Type* >& argtypes, MESSAGE& message) {
	vector<Dist> dists;
	pair<multimap<string,Function*>::iterator,multimap<string,Function*>::iterator> pa = functions.equal_range(*name);
	for (multimap<string,Function*>::iterator it = pa.first; it != pa.second; it++) {
		// TODO This has bad asymptotic behaviour O(N) when there are N functions with the same name
		Function * func = it->second;
		if (func->parameters->size() != argtypes.size())
			continue;
		dists.push_back(Dist(func, vector<int>()));
		vector<int> & dv = dists.back().second;
		bool ok = true;
		for (int i = 0; i < argtypes.size(); i++) {
			dv.push_back(argtypes[i]->distance((*func->parameters)[i]->type->real()));
			if (dv.back() == INFTY) {
				ok = false;
				break;
			}
		}
		if (!ok)
			dists.pop_back();
	}
	if (dists.empty()) {
		message = NONEFOUND;
		return 0;
	}
	vector<int> smallest(argtypes.size(), INFTY);
	for (int i = 0; i < dists.size(); i++) {
		for (int k = 0; k < argtypes.size(); k++) {
			smallest[k] = min(smallest[k], dists[i].second[k]);
		}
	}
	Function *func = 0;
	for (int i = 0; i < dists.size(); i++) {
		bool ok = true;
		for (int k = 0; k < argtypes.size(); k++) {
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
