#include <stdio.h>
#include <assert.h>
#include "flowgraph.h"
#include "runner.h"

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
	ssamaxphi = -1;
	ssainqueue = -1;
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
		paridinchild.push_back(suc[i]->par.size());
		suc[i]->searchNodes(g, this);
		//fprintf(stderr, "%d -> %d\n", num, suc[i]->num);
	}
}

// TODO implement balanced tree for faster eval, link
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
		w->dom->domsuc.push_back(w);
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

void Graph::convertToSSA() {
	vars.resize(varnum);
	for (int i = 0; i < varnum; i++)
		vars[i] = new Variable;
	for (int i = 0; i < nodes.size(); i++) {
		Node *v = nodes[i];
		if (v->par.size() >= 2) {
			for (int k = 0; k < v->par.size(); k++) {
				Node *p = v->par[k];
				while(p != v->dom) {
					p->domfront.push_back(v);
					p = p->dom;
				}
			}
		}
		for (int k = 0; k < v->args.size(); k++) {
			Arg *a = v->args[k];
			if (a->argtype == SETARG)
				vars[a->value]->setters.push_back(v);
		}
	}
	queue<Node*> qu;
	for (int i = 0; i < varnum; i++) {
		for (int k = 0; k < vars[i]->setters.size(); k++) {
			Node *a = vars[i]->setters[k];
			qu.push(a);
			a->ssainqueue = i;
		}
		while(!qu.empty()) {
			Node *a = qu.front();
			qu.pop();
			for (int k = 0; k < a->domfront.size(); k++) {
				Node *b = a->domfront[k];
				if (b->ssamaxphi < i) {
					b->phis.push_back(new Phi(i, b->par.size()));
					b->ssamaxphi = i;
					for (int j = 0; j < b->domfront.size(); j++) {
						Node *c = b->domfront[j];
						if (c->ssainqueue < i) {
							qu.push(c);
							c->ssainqueue = i;
						}
					}
				}
			}
		}
	}
	start->visitSSA(*this);
}

void Node::visitSSA(Graph &g) {
	for (int i = 0; i < phis.size(); i++) {
		Variable *v = g.vars[phis[i]->varnum];
		v->aktsub.push(v->nextsub);
		phis[i]->setsub = v->nextsub++;
	}
	for (int i = 0; i < args.size(); i++) {
		if (args[i]->argtype == SETARG) {
			Variable *v = g.vars[args[i]->value];
			v->aktsub.push(v->nextsub);
			args[i]->sub = v->nextsub++;
		} else if (args[i]->argtype == GETARG) {
			int varid = args[i]->value;
			int sub = g.vars[varid]->aktsub.top();
			args[i]->sub = sub;
			while(g.vars[varid]->getters.size() <= sub)
				g.vars[varid]->getters.push_back(vector<Node*>());
			g.vars[varid]->getters[sub].push_back(this);
		}
	}
	for (int i = 0; i < suc.size(); i++) {
		Node *v = suc[i];
		for (int k = 0; k < v->phis.size(); k++) {
			v->phis[k]->getsubs[paridinchild[i]] = g.vars[v->phis[k]->varnum]->aktsub.top();
		}
	}
	for (int i = 0; i < domsuc.size(); i++)
		domsuc[i]->visitSSA(g);
	for (int i = 0; i < phis.size(); i++) {
		Variable *v = g.vars[phis[i]->varnum];
		v->aktsub.pop();
	}
	for (int i = 0; i < args.size(); i++) {
		if (args[i]->argtype == SETARG) {
			Variable *v = g.vars[args[i]->value];
			v->aktsub.pop();
		}
	}
}

void Graph::constantPropagation() {
	fprintf(stderr, "constprop\n");
	queue<Node*> qu;
	for (int i = 0; i < nodes.size(); i++) {
		Node *a = nodes[i];
		if (!opconst(a->op)) {
			a->unconstgets = INFTY;
		} else {
			a->unconstgets = 0;
			for (int k = 0; k < a->args.size(); k++) {
				if (a->args[k]->argtype == GETARG)
					a->unconstgets++;
			}
			if (a->unconstgets == 0) {
				qu.push(a);
			}
		}
	}
	while(!qu.empty()) {
		Node *a = qu.front();
		qu.pop();
		a->eval(*this, qu);
	}
}

void Node::eval(Graph& g, queue<Node*> &qu) {
	vector<int> resultpos;
	resultpos.push_back(0);
	vector<int> resultsize;
	resultsize.push_back(0);
	vector<vector<int> > liste;
	liste.push_back(vector<int>());
	liste[0].push_back(op);
	vector<vector<int> > stops;
	stops.push_back(vector<int>());
	vector<vector<int> > argsizes;
	argsizes.push_back(vector<int>());
	state stat;
	stat.hash.push_back(0); // Phantom entry to ensure that every pointer is > 0
	stat.hashispointer.push_back(false);
	
	stat.stac.push_back(new stackentry());
	stat.stac.back()->aktpos = 0;
	stat.stac.back()->funcnum = 0;
	stat.stac.back()->copyresultto = -1;
	
	int setcount = 0;
	for (int i = 0; i < args.size(); i++) {
		if (args[i]->argtype == SETARG) {
			liste[0].push_back(stat.stac.back()->regs.size());
			for (int k = 0; k < args[i]->len; k++) {
				stat.stac.back()->regs.push_back(0);
				stat.stac.back()->ispointer.push_back(false);
			}
			setcount++;
		} else if (args[i]->argtype == GETARG) {
			int varid = args[i]->value;
			int sub = args[i]->sub;
			liste[0].push_back(stat.stac.back()->regs.size());
			stat.stac.back()->regs.push_back(g.vars[varid]->consts[sub]); // TODO support fields with length > 1
			stat.stac.back()->ispointer.push_back(false);
			for (int k = 1; k < args[i]->len; k++) {
				stat.stac.back()->regs.push_back(0);
				stat.stac.back()->ispointer.push_back(false);
			}
		} else {
			liste[0].push_back(args[i]->value);
		}
	}
	run(resultpos, resultsize, liste, stops, argsizes, stat);
	int stacpos = 0;
	for (int i = 0; i < args.size(); i++) {
		if (args[i]->argtype == SETARG) {
			int varid = args[i]->value;
			int sub = args[i]->sub;
			for (int k = 0; k < g.vars[varid]->getters[sub].size(); k++) {
				Node *b = g.vars[varid]->getters[sub][k];
				b->unconstgets--;
				if (b->unconstgets == 0)
					qu.push(b);
			}
			while(g.vars[varid]->consts.size() <= sub)
				g.vars[varid]->consts.push_back(0);
			g.vars[varid]->consts[sub] = stat.stac.back()->regs[stacpos]; // TODO support fields with length > 1
			fprintf(stderr, "const: %d_%d = %d\n", varid, sub, g.vars[varid]->consts[sub]);
		}
		stacpos += args[i]->len;
	}
	
	// TODO support functions with multiple return values or return types != INT
	if (setcount == 1) {
		int varid;
		int sub;
		for (int i = 0; i < args.size(); i++) {
			if (args[i]->argtype == SETARG) {
				varid = args[i]->value;
				sub = args[i]->sub;
			}
		}
		op = INT_CONST;
		args.clear();
		args.push_back(new Arg(INTARG, g.vars[varid]->consts[sub]));
		args.push_back(new Arg(SETARG, varid, 1));
		args[1]->sub = sub;
	}
}

void Graph::deadCodeElimination() {
	for (int i = 0; i < vars.size(); i++) {
		for (int k = 0; k < vars[i]->getters.size(); k++)
			vars[i]->getters[k].clear();
	}
	for (int k = 0; k < nodes.size(); k++) {
		Node *a = nodes[k];
		for (int i = 0; i < a->args.size(); i++) {
			if (a->args[i]->argtype == GETARG) {
				int varid = a->args[i]->value;
				int sub = a->args[i]->sub;
				while(vars[varid]->getters.size() <= sub)
					vars[varid]->getters.push_back(vector<Node*>());
				vars[varid]->getters[sub].push_back(a);
			}
		}
	}
	for (int k = 0; k < nodes.size(); k++) {
		Node *a = nodes[k];
		if (!opconst(a->op))
			continue;
		bool needed = false;
		for (int i = 0; i < a->args.size(); i++) {
			if (a->args[i]->argtype == SETARG) {
				int varid = a->args[i]->value;
				int sub = a->args[i]->sub;
				if (vars[varid]->getters.size() <= sub || !vars[varid]->getters[sub].empty())
					needed = true;
			}
		}
		if (!needed) {
			a->removeNode();
		}
	}
}

void Node::removeNode() {
	assert(suc.size() == 1 && !par.empty());
	for (int i = 0; i < par.size(); i++) {
		Node *p = par[i];
		for (int r = 0; r < p->suc.size(); r++) {
			if (p->suc[r] == this) {
				p->suc[r] = suc[0];
			}
		}
		bool found = false;
		for (int r = 0; r < suc[0]->par.size(); r++) {
			if (suc[0]->par[r] == this) {
				suc[0]->par[r] = p;
				found = true;
				break;
			}
		}
		if (!found)
			suc[0]->par.push_back(p);
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
		for (int i = 0; i < aktnode->phis.size(); i++) {
			Phi *p = aktnode->phis[i];
			fprintf(fi, "%d_%d <= phi(", p->varnum, p->setsub);
			for (int k = 0; k < p->getsubs.size(); k++) {
				if (k > 0)
					fprintf(fi, ", ");
				fprintf(fi, "%d_%d", p->varnum, p->getsubs[k]);
			}
			fprintf(fi, ")\\n");
		}
		fprintf(fi, "%s (", opname(aktnode->op).c_str());
		for (int i = 0; i < aktnode->args.size(); i++) {
			if (i > 0)
				fprintf(fi, ",");
			fprintf(fi, "%d", aktnode->args[i]->value);
			if (aktnode->args[i]->argtype == GETARG || aktnode->args[i]->argtype == SETARG)
				fprintf(fi, "_%d", aktnode->args[i]->sub);
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
