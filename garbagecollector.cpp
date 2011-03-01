
#include <stdio.h>
#include <assert.h>
#include <queue>
#include "garbagecollector.h"

//#define PRINTALLOC
//#define PRINTGC

void free(int i, state &st) {
#ifdef PRINTALLOC
	printf("delete %d-%d\n", i, i+2+st.hash[i]);
#endif
	int len = st.hash[i];
	// 0 - FREE, 1 - ALLOCATED, 2 - UNREACHABLE
	st.hash[i+1] = 0;
	if (i+4+len < st.hash.size() && st.hash[i+4+len] == 0) { // next block is free too
		st.hash[i] += st.hash[i+3+len]+3;
		len += st.hash[i+3+len]+3;
		st.hash[i+2+len] = len;
	}
	if (i > 1) {
		int vlen = st.hash[i-1];
		if(st.hash[i-2-vlen] == 0) {
			len += vlen+3;
			st.hash[i-3-vlen] = len;
			i = i-3-vlen;
			st.hash[i+2+len] = len;
		}
	}
	while(st.freeblocks.size() <= len)
		st.freeblocks.push_back(vector<int>());
	st.freeblocks[len].push_back(i);
}

int alloc(int len, state& st) {
	int foundlen = len;
	while(true) {
		if (foundlen >= st.freeblocks.size()) {
			int pos = st.hash.size();
			st.hash.push_back(len);
			st.hashispointer.push_back(false);
			st.hash.push_back(1);
			st.hashispointer.push_back(false);
			for (int i = 0; i < len; i++) {
				st.hash.push_back(0);
				st.hashispointer.push_back(false);
			}
			st.hash.push_back(len);
			st.hashispointer.push_back(false);
#ifdef PRINTALLOC
			printf("all %d-%d\n", pos, pos+2+len);
#endif
			return pos+2;
		} else {
			while(!st.freeblocks[foundlen].empty()) {
				int pos = st.freeblocks[foundlen].back();
				st.freeblocks[foundlen].pop_back();
				if (st.hash[pos] == foundlen && st.hash[pos+1] == 0) {
					st.hash[pos+1] = 1;
					for (int i = 0; i < len; i++) {
						st.hash[pos+2+i] = 0;
						st.hashispointer[pos+2+i] = false;
					}
					if (foundlen > len) {
						st.hash[pos] = len;
						st.hash[pos+2+len] = len;
						st.hash[pos+3+len] = foundlen-len-3;
						st.hash[pos+2+foundlen] = foundlen-len-3;
					}
#ifdef PRINTALLOC
					printf("all %d-%d\n", pos, pos+2+len);
#endif
					return pos+2;
				}
			}
		}
		if (foundlen == len)
			foundlen += 3;
		else
			foundlen++;
	}
}

void gc(state& st) {
	for (int pos = 1; pos < st.hash.size(); ) {
		if (st.hash[pos+1] == 1)
			st.hash[pos+1] = 2;
		pos = pos+3+st.hash[pos];
	}
	queue<int> qu;
#ifdef PRINTGC
	printf("from stack: ");
#endif
	for (int a = 0; a < st.stac.size(); a++) {
		const stackentry & se = *st.stac[a];
		for (int b = 0; b < se.regs.size(); b++) {
			if (se.ispointer[b]) {
				int p = se.regs[b];
				if (p > 0 && st.hash[p-1] == 2) {
					st.hash[p-1] = 1;
					qu.push(p);
#ifdef PRINTGC
					printf("%d ", p);
#endif
				}
			}
		}
	}
#ifdef PRINTGC
	printf("\nfrom hash: ");
#endif
	while(!qu.empty()) {
		int a = qu.front();
#ifdef PRINTGC
		printf("%d ", a);
#endif
		qu.pop();
		for (int b = 0; b < st.hash[a-2]; b++) {
			if (st.hashispointer[a+b]) {
				int p = st.hash[a+b];
				if (p > 0 && st.hash[p-1] == 2) {
					st.hash[p-1] = 1;
					qu.push(p);
				}
			}
		}
	}
#ifdef PRINTGC
	printf("\n");
#endif
	for (int pos = 1; pos < st.hash.size(); ) {
		int npos = pos+3+st.hash[pos];
		if (st.hash[pos+1] == 2) {
			free(pos, st);
		}
		pos = npos;
	}
}
