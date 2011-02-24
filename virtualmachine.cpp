
#include <stdio.h>
#include <vector>
#include <stack>
#include <string.h>
using namespace std;

// FIXME Buffer overflow
char input[10000];
//int liste[50000];
vector<vector<int> > liste;

vector<vector<int> > stops;
vector<vector<int> > argsizes;

struct stackentry {
	int funcnum;
	int aktpos;
	vector<int> regs;
};

stack<stackentry> stac;
vector<int> hash;

char opcodes[] = {'A','I','+','-','*','/','%','C','P','R','S','G','J','j','c'};
int oplength[] = { 1,  2,  3,  3,  3,  3,  3,  3,  2,  2,  3 , 3 , 2,  1 , 1 };

int main(int argc, char *argv[]) {
	if (argc >= 2)
		freopen(argv[1], "r", stdin);
	
	int L;
	for (L = 0; true; L++) {
		int c = fgetc(stdin);
		if (c == EOF)
			break;
		if (c == '\n')
			L--;
		else if (c == '|')
			input[L] = '\0';
		else
			input[L] = c;
	}
	input[L] = '\0';
	
	int N = 0;
	char *codepos = input;
	while(*codepos != '\0') {
		char op = *codepos;
		
		if (op == 'M') {
			int stop;
			sscanf(codepos+1, "%d", &stop);
			while(stops.back().size() <= stop)
				stops.back().push_back(0);
			stops.back()[stop] = N;
		} else if (op == 'F') {
			liste.push_back(vector<int>());
			stops.push_back(vector<int>());
			argsizes.push_back(vector<int>());
			N = 0;
			int anzparams;
			char *pos;
			for (pos = codepos; *pos != '\0'; pos++)
				if (*pos == ';')
					*pos = '\0';
			sscanf(codepos+1, "%d", &anzparams);
			char *npos = codepos+strlen(codepos)+1;
			for (int i = 0; i < anzparams; i++) {
				int s;
				sscanf(npos, "%d", &s);
				//printf("Farg %d\n", s);
				argsizes.back().push_back(s);
				npos += strlen(npos)+1;
			}
			for (pos--; pos != codepos+1; pos--)
				if (*pos == '\0')
					*pos = ';';
		} else {
			liste.back().push_back(-1);
			for (int i = 0; i < sizeof(opcodes)/sizeof(char); i++)
				if (opcodes[i] == op)
					liste.back()[N] = i;
			if (liste.back()[N] == -1 && op != 'M')
				fprintf(stderr, "Unknown command '%c'!\n", op);
			if (op == 'A' || op == 'j') {
				liste.back().push_back(0);
				sscanf(codepos+1, "%d", &liste.back()[N+1]);
				N += 2;
			} else if (op == 'I' || op == 'P' || op == 'R' || op == 'J') {
				liste.back().push_back(0); liste.back().push_back(0);
				sscanf(codepos+1, "%d;%d", &liste.back()[N+1], &liste.back()[N+2]);
				N += 3;
			} else if (op == '+' || op == '-' || op == '*' || op == '/' || op == '%' || op == 'C' || op == 'S' || op == 'G') {
				liste.back().push_back(0); liste.back().push_back(0); liste.back().push_back(0);
				sscanf(codepos+1, "%d;%d;%d", &liste.back()[N+1], &liste.back()[N+2], &liste.back()[N+3]);
				N += 4;
			} else if (op == 'c') {
				int func;
				char *pos;
				for (pos = codepos; *pos != '\0'; pos++)
					if (*pos == ';')
						*pos = '\0';
				sscanf(codepos+1, "%d", &func);
				liste.back().push_back(func);
				N += 2;
				char *npos = codepos+strlen(codepos)+1;
				for (int i = 0; npos != pos+1; i++) {
					int s;
					sscanf(npos, "%d", &s);
					//printf("carg %d\n", s);
					liste.back().push_back(s);
					npos += strlen(npos)+1;
					N++;
				}
				for (pos--; pos != codepos+1; pos--)
					if (*pos == '\0')
						*pos = ';';
			} else {
				fprintf(stderr, "Unknown command '%c'!\n", op);
			}
		}
		codepos += strlen(codepos)+1;
	}
	
	//int aktpos = 0;
	//while(aktpos != N) {
	stac.push(stackentry());
	stac.top().aktpos = 0;
	stac.top().funcnum = 0;
	
	while(!stac.empty()) {
		stackentry & se = stac.top();
		vector<int> & st = se.regs;
		vector<int> & sto = stops[se.funcnum];
		vector<int> & li = liste[se.funcnum];
		int lisize = li.size();
		int aktpos = se.aktpos;
		
		while(aktpos < lisize) {
			char op = li[aktpos];
			int len, co, pos, posa, posb, posc;
			int nextpos;
			aktpos++;
			switch(op) {
				case 0:
					len = li[aktpos];
					if (len > 0) {
						for (int i = 0; i < len; i++)
							st.push_back(0);
					} else {
						for (int i = 0; i < -len; i++) {
							st.pop_back();
						}
					}
					nextpos = aktpos+1;
					break;
				case 1:
					co = li[aktpos]; pos = li[aktpos+1];
					st[li[aktpos+1]] = li[aktpos];
					nextpos = aktpos+2;
					break;
				case 2:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa]+st[posb];
					nextpos = aktpos+3;
					break;
				case 3:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa]-st[posb];
					nextpos = aktpos+3;
					break;
				case 4:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa]*st[posb];
					nextpos = aktpos+3;
					break;
				case 5:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa]/st[posb];
					nextpos = aktpos+3;
					break;
				case 6:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa]%st[posb];
					nextpos = aktpos+3;
					break;
				case 7:
					posa = li[aktpos]; len = li[aktpos+1]; posb = li[aktpos+2];
					for (int i = 0; i < len; i++)
						st[posb+i] = st[posa+i];
					nextpos = aktpos+3;
					break;
				case 8:
					pos = li[aktpos]; len = li[aktpos+1];
					for (int i = 0; i < len; i++)
						printf("%d ", st[pos+i]);
					printf("\n");
					nextpos = aktpos+2;
					break;
				case 9:
					pos = li[aktpos]; len = li[aktpos+1];
					st[pos] = hash.size();
					for (int i = 0; i < len; i++)
						hash.push_back(0);
					nextpos = aktpos+2;
					break;
				case 10:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					hash[st[posb]+posc] = st[posa];
					nextpos = aktpos+3;
					break;
				case 11:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = hash[st[posa]+posb];
					nextpos = aktpos+3;
					break;
				case 12:
					posa = li[aktpos]; posb = li[aktpos+1];
					if (st[posa] == 0)
						nextpos = sto[posb];
					else
						nextpos = aktpos+2;
					break;
				case 13:
					posa = li[aktpos];
					nextpos = sto[posa];
					break;
				case 14:
					co = li[aktpos];
					stac.push(stackentry());
					stac.top().aktpos = 0;
					stac.top().funcnum = co;
					se.aktpos = aktpos;
					for (int i = 0; i < argsizes[co].size(); i++) {
						for (int k = 0; k < argsizes[co][i]; k++) {
							//printf("%d -> %d\n", li[aktpos+i+1]+k, stac.top().regs.size());
							stac.top().regs.push_back(st[li[aktpos+i+1]+k]);
						}
						se.aktpos++;
					}
					/*printf("called: ");
					for (int r = 0; r < stac.top().regs.size(); r++)
						printf("%d ", stac.top().regs[r]);
					printf("\n");*/
					se.aktpos++;
					goto stackup;
					break;
				default:
					fprintf(stderr, "Unknown command %d!\n", op);
					return 0;
			}
			/*printf("%d %c (", stac.top().funcnum, opcodes[op]);
			for (int i = 0; i < oplength[op]; i++) {
				printf("%2d", li[aktpos+i]);
				if (i < oplength[op]-1)
					printf(",");
			}
			printf("):\t ");
			for (int i = 0; i < st.size(); i++) {
				printf("%d ", st[i]);
			}
			printf("\n");
			printf("   ");
			for (int i = 0; i < hash.size(); i++) {
				printf("%d ", hash[i]);
			}
			printf("\n");*/
			aktpos = nextpos;
		}
		stac.pop();
		//se.aktpos = aktpos;
stackup:	;
	}
	//if (stack.size() != 0)
	//	fprintf(stderr, "Stack not cleared!\n");
	return 0;
}
