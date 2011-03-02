
INSTRUCTION(FUNC,0,)
INSTRUCTION(FUNC_MAIN,0,)
INSTRUCTION(HERE_STOP,0,)

// Allocate on stack
INSTRUCTION(ALLOC_STACK,1,
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
)

// Integer constant
INSTRUCTION(INT_CONST,2,
	co = li[aktpos]; pos = li[aktpos+1];
	st[li[aktpos+1]] = li[aktpos];
	ip[li[aktpos+1]] = false;
	nextpos = aktpos+2;
)

// Copy on stack
INSTRUCTION(COPY_STACK,3,
	posa = li[aktpos]; len = li[aktpos+1]; posb = li[aktpos+2];
	for (int i = 0; i < len; i++) {
		st[posb+i] = st[posa+i];
		ip[posb+i] = ip[posa+i];
	}
	nextpos = aktpos+3;
)

// Print
INSTRUCTION(PRINT,2,
	pos = li[aktpos]; len = li[aktpos+1];
	for (int i = 0; i < len; i++)
		printf("%d ", st[pos+i]);
	printf("\n");
	nextpos = aktpos+2;
)

// Allocate fixed amount on heap and save pointer
INSTRUCTION(ALLOC_HEAP_CONSTAMOUNT,2,
	pos = li[aktpos]; len = li[aktpos+1];
	st[pos] = alloc(len, stat);
	ip[pos] = true;
	nextpos = aktpos+2;
)

// Allocate variable amount on heap and save pointer
INSTRUCTION(ALLOC_HEAP_VARAMOUNT,3,
	len = li[aktpos]; posa = li[aktpos+1]; posb = li[aktpos+2];
	st[posb] = alloc(len*st[posa], stat);
	ip[posb] = true;
	nextpos = aktpos+3;
)

// Set on heap
INSTRUCTION(SET_HEAP,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	stat.hash[st[posb]+posc] = st[posa];
	stat.hashispointer[st[posb]+posc] = ip[posa];
	nextpos = aktpos+3;
)

// Get from heap
INSTRUCTION(GET_HEAP,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = stat.hash[st[posa]+posb];
	ip[posc] = stat.hashispointer[st[posa]+posb];
	nextpos = aktpos+3;
)

// Set on heap array
INSTRUCTION(SET_ARRAY,4,
	len = li[aktpos]; posa = li[aktpos+1]; posb = li[aktpos+2]; posc = li[aktpos+3];
	for (int i = 0; i < len; i++) {
		stat.hash[st[posb]+len*st[posc]+i] = st[posa+i];
		stat.hashispointer[st[posb]+len*st[posc]+i] = ip[posa+i];
	}
	nextpos = aktpos+4;
)

// Get from heap array
INSTRUCTION(GET_ARRAY,4,
	len = li[aktpos]; posa = li[aktpos+1]; posb = li[aktpos+2]; posc = li[aktpos+3];
	for (int i = 0; i < len; i++) {
		st[posc+i] = stat.hash[st[posa]+len*st[posb]+i];
		ip[posc+i] = stat.hashispointer[st[posa]+len*st[posb]+i];
	}
	nextpos = aktpos+4;
)

// Jump if
INSTRUCTION(JUMPIF,2,
	posa = li[aktpos]; posb = li[aktpos+1];
	if (st[posa] == 0)
		nextpos = sto[posb];
	else
		nextpos = aktpos+2;
)

// Jump
INSTRUCTION(JUMP,1,
	posa = li[aktpos];
	nextpos = sto[posa];
)

// Call function
INSTRUCTION(CALL,1,
	co = li[aktpos]; pos = li[aktpos+1];
	stat.stac.push_back(new stackentry());
	stat.stac.back()->aktpos = 0;
	stat.stac.back()->funcnum = co;
	stat.stac.back()->copyresultto = pos;
	se.aktpos = aktpos;
	for (int i = 0; i < argsizes[co].size(); i++) {
		for (int k = 0; k < argsizes[co][i]; k++) {
			stat.stac.back()->regs.push_back(st[li[aktpos+i+2]+k]);
			stat.stac.back()->ispointer.push_back(ip[li[aktpos+i+2]+k]);
		}
		se.aktpos++;
	}
	/*printf("called: (%d): ", stat.stac.back()->regs.size());
	for (int r = 0; r < stat.stac.back()->regs.size(); r++)
		printf("%d ", stat.stac.back()->regs[r]);
	printf("\n");*/
	se.aktpos+=2;
	goto stackup;
)

// Dump stack
INSTRUCTION(DUMP_STACK,0,
	printf("stack: ");
	for (int i = 0; i < st.size(); i++) {
		if (ip[i])
			printf("p");
		printf("%d ", st[i]);
	}
	printf("\n");
	nextpos = aktpos;
)

// Dump heap
INSTRUCTION(DUMP_HEAP,0,
	printf("heap: ");
	for (int i = 0; i < stat.hash.size(); i++) {
		if (stat.hashispointer[i])
			printf("p");
		printf("%d ", stat.hash[i]);
	}
	printf("\n");
	nextpos = aktpos;
)

INSTRUCTION(PLUS,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa]+st[posb];
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(MINUS,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa]-st[posb];
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(TIMES,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa]*st[posb];
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(DIV,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa]/st[posb];
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(MOD,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa]%st[posb];
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(EQUAL,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] == st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(LESS,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] < st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(GREATER,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] > st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(LESSOREQUAL,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] <= st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(GREATEROREQUAL,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] >= st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(UNEQUAL,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = st[posa] != st[posb] ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(AND,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = (st[posa]!=0 && st[posb]!=0 ? 1 : 0);
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(OR,3,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	st[posc] = (st[posa]!=0 || st[posb]!=0 ? 1 : 0);
	ip[posc] = false;
	nextpos = aktpos+3;
)
