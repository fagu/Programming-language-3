#include <stdio.h>
#include <assert.h>
#include "flowgraph.h"

void Graph::addNewStops() {
	nextstopid = 0;
	nextnum = 0;
	start->removeStops();
	start->searchNodes(*this);
	start->addGoodStops(*this);
	vector<Node*> withouts;
	for (int i = 0; i < nodes.size(); i++)
		if (!nodes[i]->outputprefound)
			withouts.push_back(nodes[i]);
	for (int i = 0; i < withouts.size()-1; i++) {
		Node *a = withouts[i];
		while(a->outputsuc)
			a = a->outputsuc;
		a->outputsuc = withouts[i+1];
		withouts[i+1]->outputprefound = true;
	}
}

void Graph::printGraph(FILE *fi) {
	fprintf(fi, "digraph G {\n");
	start->printGraph(*this, fi);
	fprintf(fi, "}\n");
}

void Node::init(OPCODE _op) {
	op = _op;
	found = false;
	inrem = false;
	ingood = false;
	instack = false;
	graphprinted = false;
	stopid = -1;
	outputsuc = 0;
	outputprefound = false;
}

void Node::removeStops() {
	if (inrem)
		return;
	inrem = true;
	for (int i = 0; i < suc.size(); i++) {
		suc[i]->removeStops();
		if (suc[i]->op == HERE_STOP) {
			suc[i] = suc[i]->suc[0];
		}
	}
}

void Node::searchNodes(Graph& g) {
	if (found)
		return;
	//num = g.nextnum++;
	//fprintf(stderr, "%d: %d\n", num, op);
	found = true;
	g.nodes.push_back(this);
	for (int i = 0; i < suc.size(); i++) {
		suc[i]->searchNodes(g);
		//fprintf(stderr, "%d -> %d\n", num, suc[i]->num);
	}
}

void Node::addGoodStops(Graph& g) {
	if (ingood)
		return;
	ingood = true;
	instack = true;
	for (int i = 0; i < suc.size(); i++) {
		if (!suc[i]->outputprefound && !suc[i]->instack && (op != JUMPIF || i == 1)) {
			outputsuc = suc[i];
			outputsuc->outputprefound = true;
			break;
		}
	}
	for (int i = 0; i < suc.size(); i++) {
		if (outputsuc != suc[i]) {
			if (suc[i]->stopid == -1)
				suc[i]->stopid = g.nextstopid++;
			//fprintf(stderr, "s%d\n", suc[i]->stopid);
		}
	}
	for (int i = 0; i < suc.size(); i++)
		suc[i]->addGoodStops(g);
	instack = false;
}

void Node::print() {
	if (stopid != -1)
		printf("%d;%d;\n", HERE_STOP, stopid);
	assert((suc.size() == 0 && op == RETURN) || (suc.size() == 1 && op != JUMPIF && op != RETURN) || (suc.size() == 2 && op == JUMPIF));
	if (op != VIRTUAL_START) {
		printf("%d;", op);
		for (int i = 0; i < args.size(); i++)
			printf("%d;", args[i]->value);
		if (op == JUMPIF)
			printf("%d;", suc[0]->stopid);
		printf("\n");
	}
	if (!suc.empty())
		if (outputsuc != suc.back())
			printf("%d;%d;\n", JUMP, suc.back()->stopid);
	if (outputsuc)
		outputsuc->print();
}

void Node::printGraph(Graph &g, FILE *fi) {
	if (graphprinted)
		return;
	graphprinted = true;
	num = g.nextnum++;
	for (int i = 0; i < suc.size(); i++) {
		suc[i]->printGraph(g, fi);
		fprintf(fi, "\t%d -> %d;\n", num, suc[i]->num);
	}
}
