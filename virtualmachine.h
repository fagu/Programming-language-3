
#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <vector>
#include <stack>
#include <list>
using namespace std;

struct stackentry {
	int funcnum;
	int aktpos;
	vector<int> regs;
	vector<bool> ispointer;
	int copyresultto;
};

struct state {
	vector<stackentry*> stac;
	vector<int> hash;
	vector<bool> hashispointer;
	vector<vector<int> > freeblocks; // TODO this is very inefficient!!!
};

#endif // VIRTUALMACHINE_H