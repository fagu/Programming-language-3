#ifndef RUNNER_H
#define RUNNER_H

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

void run(vector<int> &resultpos, vector<int> &resultsize, vector<vector<int> > &liste, vector<vector<int> > &stops, vector<vector<int> > &argsizes, state &stat);

#endif // RUNNER_H