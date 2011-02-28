
#include <stdio.h>
#include <string.h>
#include "virtualmachine.h"
#include "garbagecollector.h"

//#define PRINTHASH
//#define PRINTSTACK

// FIXME Buffer overflow
char input[10000];
int mainfunc = -1;
vector<vector<int> > liste;

vector<vector<int> > stops;
vector<vector<int> > argsizes;

char opcodes[] = {'A','I','+','-','*','/','%','C','P','R','S','G','J','j','c','=','<','>','(',')','~','&','O'};
int oplength[] = { 1,  2,  3,  3,  3,  3,  3,  3,  2,  2,  3 , 3 , 2,  1 , 1 , 3 , 3 , 3 , 3 , 3 , 3 , 3 , 3 };

state stat;

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
		} else if (op == 'F' || op == 'f') {
			if (op == 'F')
				mainfunc = liste.size();
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
			} else if (op == '+' || op == '-' || op == '*' || op == '/' || op == '%' || op == '=' || op == '<' || op == '>' || op == '(' || op == ')' || op == '~' || op == '&' || op == 'O' || op == 'C' || op == 'S' || op == 'G') {
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
	
	if (mainfunc == -1)
		fprintf(stderr, "No main function specified!\n");
	
	stat.hash.push_back(0); // Phantom entry to ensure that every pointer is > 0
	stat.hashispointer.push_back(false);
	
	stat.stac.push_back(new stackentry());
	stat.stac.back()->aktpos = 0;
	stat.stac.back()->funcnum = mainfunc;
	
	while(!stat.stac.empty()) {
		stackentry & se = *stat.stac[stat.stac.size()-1];
		vector<int> & st = se.regs;
		vector<bool> & ip = se.ispointer;
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
						for (int i = 0; i < len; i++) {
							st.push_back(0);
							ip.push_back(false);
						}
					} else {
						for (int i = 0; i < -len; i++) {
							st.pop_back();
							ip.push_back(false);
						}
					}
					nextpos = aktpos+1;
					break;
				case 1:
					co = li[aktpos]; pos = li[aktpos+1];
					st[li[aktpos+1]] = li[aktpos];
					ip[li[aktpos+1]] = false;
					nextpos = aktpos+2;
					break;
				case 2:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa]+st[posb];
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 3:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa]-st[posb];
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 4:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa]*st[posb];
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 5:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa]/st[posb];
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 6:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa]%st[posb];
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 7:
					posa = li[aktpos]; len = li[aktpos+1]; posb = li[aktpos+2];
					for (int i = 0; i < len; i++) {
						st[posb+i] = st[posa+i];
						ip[posb+i] = ip[posa+i];
					}
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
					st[pos] = alloc(len, stat);
					ip[pos] = true;
					nextpos = aktpos+2;
					break;
				case 10:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					stat.hash[st[posb]+posc] = st[posa];
					stat.hashispointer[st[posb]+posc] = ip[posa];
					nextpos = aktpos+3;
					break;
				case 11:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = stat.hash[st[posa]+posb];
					ip[posc] = stat.hashispointer[st[posa]+posb];
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
					stat.stac.push_back(new stackentry());
					stat.stac.back()->aktpos = 0;
					stat.stac.back()->funcnum = co;
					se.aktpos = aktpos;
					for (int i = 0; i < argsizes[co].size(); i++) {
						for (int k = 0; k < argsizes[co][i]; k++) {
							stat.stac.back()->regs.push_back(st[li[aktpos+i+1]+k]);
							stat.stac.back()->ispointer.push_back(ip[li[aktpos+i+1]+k]);
						}
						se.aktpos++;
					}
					/*printf("called: (%d): ", stat.stac.back()->regs.size());
					for (int r = 0; r < stat.stac.back()->regs.size(); r++)
						printf("%d ", stat.stac.back()->regs[r]);
					printf("\n");*/
					se.aktpos++;
					goto stackup;
					break;
				case 15:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa] == st[posb] ? 1 : 0;
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 16:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa] < st[posb] ? 1 : 0;
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 17:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa] > st[posb] ? 1 : 0;
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 18:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa] <= st[posb] ? 1 : 0;
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 19:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa] >= st[posb] ? 1 : 0;
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 20:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = st[posa] != st[posb] ? 1 : 0;
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 21:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = (st[posa]!=0 && st[posb]!=0 ? 1 : 0);
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				case 22:
					posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
					st[posc] = (st[posa]!=0 || st[posb]!=0 ? 1 : 0);
					ip[posc] = false;
					nextpos = aktpos+3;
					break;
				default:
					fprintf(stderr, "Unknown command %d!\n", op);
					return 0;
			}
#ifdef PRINTSTACK
			printf("%d %c (", stat.stac.back()->funcnum, opcodes[op]);
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
#endif
#ifdef PRINTHASH
			printf("   ");
			for (int i = 0; i < stat.hash.size(); i++) {
				printf("%d ", stat.hash[i]);
			}
			printf("\n");
#endif
			aktpos = nextpos;
			gc(stat);
		}
		stat.stac.pop_back();
stackup:	;
	}
	return 0;
}
