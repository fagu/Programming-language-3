#include <stdio.h>
#include <assert.h>
#include "flowgraph.h"

void Node::init(OPCODE _op) {
	op = _op;
	inrem = false;
	ingood = false;
	instack = false;
	graphprinted = false;
	num = -1;
	stopid = -1;
	outputsuc = 0;
	outputprefound = false;
	inlivequeue = false;
	ancestor = 0;
	label = this;
}

void Graph::removeOldStops() {
	start->removeStops();
	start->searchNodes(*this, 0);
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

void Node::searchNodes(Graph& g, Node* p) {
	if (p)
		par.push_back(p);
	if (num != -1)
		return;
	num = g.nodes.size();
	semi = num;
	//fprintf(stderr, "%d: %d\n", num, op);
	g.nodes.push_back(this);
	for (int i = 0; i < suc.size(); i++) {
		suc[i]->searchNodes(g, this);
		//fprintf(stderr, "%d -> %d\n", num, suc[i]->num);
	}
}

void Graph::buildDomTree() {
	for (int i = nodes.size()-1; i > 0; i--) {
		Node *w = nodes[i];
		for (int k = 0; k < w->par.size(); k++) {
			Node *v = w->par[k];
			Node *u = v->eval();
			w->semi = min(w->semi, u->semi);
		}
		nodes[w->semi]->bucket.push_back(w);
		Node *p = w->par[0];
		w->ancestor = p;
		for (int k = 0; k < p->bucket.size(); k++) {
			Node *v = p->bucket[k];
			Node *u = v->eval();
			if (u->semi < v->semi)
				v->dom = u;
			else
				v->dom = p;
		}
		p->bucket.clear();
	}
	nodes[0]->dom = 0;
	for (int i = 1; i < nodes.size(); i++) {
		Node *w = nodes[i];
		if (w->dom != nodes[w->semi])
			w->dom = w->dom->dom;
	}
}

Node* Node::eval() {
	if (!ancestor)
		return this;
	compress();
	return label;
}

void Node::compress() {
	if (ancestor->ancestor) {
		ancestor->compress();
		if (ancestor->label->semi < label->semi) {
			label = ancestor->label;
		}
		ancestor = ancestor->ancestor;
	}
}

void Graph::livenessAnalysis() {
	queue<Node*> qu; // TODO High asymptotic runtime
	for (int i = 0; i < nodes.size(); i++) {
		nodes[i]->initLive(*this);
		nodes[i]->inlivequeue = true;
		qu.push(nodes[i]);
	}
	int anz = 0;
	while(!qu.empty()) {
		anz++;
		Node *n = qu.front();
		qu.pop();
		if (n->updateLive()) {
			for (int k = 0; k < n->par.size(); k++) {
				if (!n->par[k]->inlivequeue) {
					n->par[k]->inlivequeue = true;
					qu.push(n->par[k]);
				}
			}
		}
	}
	//fprintf(stderr, "Liveness %d/%d\n", anz, nodes.size());
}

void Node::initLive(Graph& g) {
	liveget.resize(g.varnum);
	liveset.resize(g.varnum);
	livein.resize(g.varnum);
	liveout.resize(g.varnum);
	for (int i = 0; i < args.size(); i++) {
		if (args[i]->argtype == GETARG)
			liveget[args[i]->value] = true;
		else if (args[i]->argtype == SETARG)
			liveset[args[i]->value] = true;
	}
}

bool Node::updateLive() {
	inlivequeue = false;
	liveout.reset();
	for (int i = 0; i < suc.size(); i++) {
		liveout |= suc[i]->livein;
	}
	dynamic_bitset<> newin = liveget | (liveout - liveset);
	if (newin != livein) {
		livein = newin;
		return true;
	}
	return false;
}

void Graph::addNewStops() {
	nextstopid = 0;
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

void Graph::printGraph(FILE *fi) {
	fprintf(fi, "digraph G {\n");
	start->printGraph(*this, fi);
	fprintf(fi, "}\n");
}

void Node::printGraph(Graph &g, FILE *fi) {
	if (graphprinted)
		return;
	graphprinted = true;
	fprintf(fi, "\t%d [shape=box,label=\"", num);
	Node *aktnode = this;
	while(true) {
		fprintf(fi, "%s (", opname(aktnode->op).c_str());
		for (int i = 0; i < aktnode->args.size(); i++) {
			if (i > 0)
				fprintf(fi, ",");
			fprintf(fi, "%d", aktnode->args[i]->value);
		}
		fprintf(fi, ")\\n");
		//break;
		if (aktnode->suc.size() != 1 || aktnode->suc[0]->stopid != -1)
			break;
		aktnode = aktnode->suc[0];
	}
	fprintf(fi, "\"];\n");
	
	//if (dom)
	//	fprintf(fi, "%d -> %d [color=red];\n", dom->num, num);
	for (int i = 0; i < aktnode->suc.size(); i++) {
		aktnode->suc[i]->printGraph(g, fi);
		fprintf(fi, "\t%d:s -> %d:n", num, aktnode->suc[i]->num);
		if (aktnode->suc.size() > 1)
			fprintf(fi, " [label=\"%d\"]", i);
		fprintf(fi, ";\n");
	}
}
