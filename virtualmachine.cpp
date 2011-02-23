
#include <stdio.h>
#include <vector>
#include <string.h>
using namespace std;

// FIXME Buffer overflow
char input[10000];
int liste[50000];

vector<int> stops;

vector<int> stack;
vector<int> hash;

char opcodes[] = {'A','I','+','-','*','/','%','C','P','R','S','G','J','j'};
int oplength[] = { 1,  2,  3,  3,  3,  3,  3,  3,  2,  2,  3 , 3 , 2,  1 };

int anzahl[14];
long long zeit[14];

int main() {
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
		liste[N] = -1;
		for (int i = 0; i < sizeof(opcodes)/sizeof(char); i++)
			if (opcodes[i] == op)
				liste[N] = i;
		if (liste[N] == -1 && op != 'M')
			fprintf(stderr, "Unknown command '%c'!\n", op);
		if (op == 'A' || op == 'j') {
			sscanf(codepos+1, "%d", &liste[N+1]);
			N += 2;
		} else if (op == 'I' || op == 'P' || op == 'R' || op == 'J') {
			sscanf(codepos+1, "%d;%d", &liste[N+1], &liste[N+2]);
			N += 3;
		} else if (op == '+' || op == '-' || op == '*' || op == '/' || op == '%' || op == 'C' || op == 'S' || op == 'G') {
			sscanf(codepos+1, "%d;%d;%d", &liste[N+1], &liste[N+2], &liste[N+3]);
			N += 4;
		} else if (op == 'M') {
			int stop;
			sscanf(codepos+1, "%d", &stop);
			while(stops.size() <= stop)
				stops.push_back(0);
			stops[stop] = N;
		} else {
			fprintf(stderr, "Unknown command '%c'!\n", op);
		}
		codepos += strlen(codepos)+1;
	}
	
	int aktpos = 0;
	while(aktpos != N) {
		char op = liste[aktpos];
		
		int len, co, pos, posa, posb, posc;
		int nextpos;
		aktpos++;
		switch(op) {
			case 0:
				len = liste[aktpos];
				if (len > 0) {
					for (int i = 0; i < len; i++)
						stack.push_back(0);
				} else {
					for (int i = 0; i < -len; i++) {
						stack.pop_back();
					}
				}
				nextpos = aktpos+1;
				break;
			case 1:
				co = liste[aktpos]; pos = liste[aktpos+1];
				stack[liste[aktpos+1]] = liste[aktpos];
				nextpos = aktpos+2;
				break;
			case 2:
				posa = liste[aktpos]; posb = liste[aktpos+1]; posc = liste[aktpos+2];
				stack[posc] = stack[posa]+stack[posb];
				nextpos = aktpos+3;
				break;
			case 3:
				posa = liste[aktpos]; posb = liste[aktpos+1]; posc = liste[aktpos+2];
				stack[posc] = stack[posa]-stack[posb];
				nextpos = aktpos+3;
				break;
			case 4:
				posa = liste[aktpos]; posb = liste[aktpos+1]; posc = liste[aktpos+2];
				stack[posc] = stack[posa]*stack[posb];
				nextpos = aktpos+3;
				break;
			case 5:
				posa = liste[aktpos]; posb = liste[aktpos+1]; posc = liste[aktpos+2];
				stack[posc] = stack[posa]/stack[posb];
				nextpos = aktpos+3;
				break;
			case 6:
				posa = liste[aktpos]; posb = liste[aktpos+1]; posc = liste[aktpos+2];
				stack[posc] = stack[posa]%stack[posb];
				nextpos = aktpos+3;
				break;
			case 7:
				posa = liste[aktpos]; len = liste[aktpos+1]; posb = liste[aktpos+2];
				for (int i = 0; i < len; i++)
					stack[posb+i] = stack[posa+i];
				nextpos = aktpos+3;
				break;
			case 8:
				pos = liste[aktpos]; len = liste[aktpos+1];
				for (int i = 0; i < len; i++)
					printf("%d ", stack[pos+i]);
				printf("\n");
				nextpos = aktpos+2;
				break;
			case 9:
				pos = liste[aktpos]; len = liste[aktpos+1];
				stack[pos] = hash.size();
				for (int i = 0; i < len; i++)
					hash.push_back(0);
				nextpos = aktpos+2;
				break;
			case 10:
				posa = liste[aktpos]; posb = liste[aktpos+1]; posc = liste[aktpos+2];
				hash[stack[posb]+posc] = stack[posa];
				nextpos = aktpos+3;
				break;
			case 11:
				posa = liste[aktpos]; posb = liste[aktpos+1]; posc = liste[aktpos+2];
				stack[posc] = hash[stack[posa]+posb];
				nextpos = aktpos+3;
				break;
			case 12:
				posa = liste[aktpos]; posb = liste[aktpos+1];
				if (stack[posa] == 0)
					nextpos = stops[posb];
				else
					nextpos = aktpos+2;
				break;
			case 13:
				posa = liste[aktpos];
				nextpos = stops[posa];
				break;
			default:
				fprintf(stderr, "Unknown command %d!\n", op);
		}
		/*printf("%c (", opcodes[op]);
		for (int i = 0; i < oplength[op]; i++) {
			printf("%2d", liste[aktpos+i]);
			if (i < oplength[op]-1)
				printf(",");
		}
		printf("):\t ");
		for (int i = 0; i < stack.size(); i++) {
			printf("%d ", stack[i]);
		}
		printf("\n");
		printf("   ");
		for (int i = 0; i < hash.size(); i++) {
			printf("%d ", hash[i]);
		}
		printf("\n");*/
		aktpos = nextpos;
	}
	if (stack.size() != 0)
		fprintf(stderr, "Stack not cleared!\n");
	return 0;
}
