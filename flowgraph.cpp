#include <stdio.h>
#include <assert.h>
#include "flowgraph.h"
#include "runner.h"

// FIXME There are bugs concerning arguments of length 0 (for example the return value of a void function)

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
	firstphi = this;
}

void Node::initSimple() {
	getargs.clear();
	setargs.clear();
	for (int i = 0; i < args.size(); i++) {
		if (args[i]->argtype == GETARG)
			getargs.push_back(args[i]);
		else if (args[i]->argtype == SETARG)
			setargs.push_back(args[i]);
	}
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
	initSimple();
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

// FIXME There is a bug when a variable is set and get in the same instruction
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
		for (int k = 0; k < v->setargs.size(); k++) {
			Arg *a = v->setargs[k];
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
					Node *phi = new Node(VIRTUAL_PHI, new Arg(SETARG, i, 1)); // TODO support types with len != 1
					for (int r = 0; r < b->firstphi->par.size(); r++)
						phi->args.push_back(new Arg(GETARG, i, 1));
					phi->initSimple();
					b->insertNode(phi);
					if (b->ssamaxphi == -1)
						b->firstphi = phi;
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
	for (int i = 0; i < nodes.size(); i++)
		nodes[i]->num = -1;
	nodes.clear();
	start->searchNodes(*this, 0);
}

void Node::visitSSA(Graph &g) {
	for (int i = 0; i < setargs.size(); i++) {
		Variable *v = g.vars[setargs[i]->value];
		v->aktsub.push(v->nextsub);
		setargs[i]->sub = v->nextsub++;
		g.vars[setargs[i]->value]->setter.push_back(this);
	}
	if (op != VIRTUAL_PHI) {
		for (int i = 0; i < getargs.size(); i++) {
			int varid = getargs[i]->value;
			int sub = g.vars[varid]->aktsub.top();
			getargs[i]->sub = sub;
			while(g.vars[varid]->getters.size() <= sub)
				g.vars[varid]->getters.push_back(vector<Node*>());
			g.vars[varid]->getters[sub].push_back(this);
		}
		for (int i = 0; i < suc.size(); i++) {
			Node *v = suc[i];
			while(v->op == VIRTUAL_PHI) {
				v->getargs[paridinchild[i]]->sub = g.vars[v->setargs[0]->value]->aktsub.top();
				v = v->suc[0];
			}
		}
	}
	for (int i = 0; i < domsuc.size(); i++)
		domsuc[i]->visitSSA(g);
	for (int i = 0; i < setargs.size(); i++) {
		Variable *v = g.vars[setargs[i]->value];
		v->aktsub.pop();
	}
}

void Node::insertNode(Node* n) { // TODO This function could have bad worst-case asymptotic runtime
	n->suc.push_back(this);
	for (int i = 0; i < par.size(); i++) {
		Node *p = par[i];
		for (int k = 0; k < p->suc.size(); k++) {
			if (p->suc[k] == this) {
				p->suc[k] = n;
				p->paridinchild[k] = n->par.size();
			}
		}
		n->par.push_back(p);
	}
	n->dom = dom;
	for (int i = 0; i < dom->domsuc.size(); i++)
		if (dom->domsuc[i] == this)
			dom->domsuc[i] = n;
	dom = n;
	n->domsuc.push_back(this);
	par.clear();
	par.push_back(n);
	n->paridinchild.push_back(0);
}

void Graph::constantPropagation() {
	queue<Node*> qu;
	for (int i = 0; i < nodes.size(); i++) {
		Node *a = nodes[i];
		if (!opconst(a->op)) {
			a->unconstgets = INFTY;
		} else {
			a->unconstgets = a->getargs.size();
			if (a->unconstgets == 0)
				qu.push(a);
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
	
	for (int i = 0; i < args.size(); i++) {
		if (args[i]->argtype == SETARG) {
			liste[0].push_back(stat.stac.back()->regs.size());
			for (int k = 0; k < args[i]->len; k++) {
				stat.stac.back()->regs.push_back(0);
				stat.stac.back()->ispointer.push_back(false);
			}
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
			if (g.vars[varid]->getters.size() > sub) {
				for (int k = 0; k < g.vars[varid]->getters[sub].size(); k++) {
					Node *b = g.vars[varid]->getters[sub][k];
					b->unconstgets--;
					if (b->unconstgets == 0)
						qu.push(b);
				}
			}
			while(g.vars[varid]->consts.size() <= sub)
				g.vars[varid]->consts.push_back(0);
			g.vars[varid]->consts[sub] = stat.stac.back()->regs[stacpos]; // TODO support fields with length > 1
			//fprintf(stderr, "const: %d_%d = %d\n", varid, sub, g.vars[varid]->consts[sub]);
		}
		stacpos += args[i]->len;
	}
	
	// TODO support functions with multiple return values or return types != INT
	assert(setargs.size() == 1);
	int varid = setargs[0]->value;
	int sub = setargs[0]->sub;
	op = INT_CONST;
	args.clear();
	getargs.clear();
	setargs.clear();
	args.push_back(new Arg(INTARG, g.vars[varid]->consts[sub]));
	args.push_back(new Arg(SETARG, varid, 1));
	args[1]->sub = sub;
	setargs.push_back(args[1]);
}

void Graph::deadCodeElimination() {
	for (int i = 0; i < vars.size(); i++) {
		vars[i]->numberofgetters.assign(vars[i]->nextsub, 0);
	}
	for (int k = 0; k < nodes.size(); k++) {
		Node *a = nodes[k];
		for (int i = 0; i < a->getargs.size(); i++) {
			int varid = a->getargs[i]->value;
			int sub = a->getargs[i]->sub;
			vars[varid]->numberofgetters[sub]++;
		}
	}
	queue<Node*> qu;
	for (int k = 0; k < nodes.size(); k++) {
		Node *a = nodes[k];
		if (!opconst(a->op)) {
			a->needcount = INFTY;
			continue;
		}
		a->needcount = 0;
		for (int i = 0; i < a->setargs.size(); i++) {
			int varid = a->setargs[i]->value;
			int sub = a->setargs[i]->sub;
			a->needcount += vars[varid]->numberofgetters[sub];
		}
		if (a->needcount == 0)
			qu.push(a);
	}
	while(!qu.empty()) {
		Node *a = qu.front();
		qu.pop();
		a->removeNode(*this, qu);
	}
}

void Node::removeNode(Graph &g, queue<Node*> &qu) { // TODO This function could have bad worst-case asymptotic runtime
	assert(suc.size() == 1 && !par.empty());
	for (int i = 0; i < getargs.size(); i++) {
		if (getargs[i]->sub) {
			Variable *v = g.vars[getargs[i]->value];
			Node *b = v->setter[getargs[i]->sub];
			b->needcount--;
			if (b->needcount == 0)
				qu.push(b);
		}
	}
	op = VIRTUAL_DELETED;
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
	for (int i = 0; i < domsuc.size(); i++) {
		domsuc[i]->dom = dom;
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
	if (op < VIRTUAL_START) {
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
