
INSTRUCTION(VIRTUAL_START,0,false,)
INSTRUCTION(VIRTUAL_PHI,0,true,)
INSTRUCTION(VIRTUAL_DELETED,0,false,)
INSTRUCTION(FUNC,0,false,)
INSTRUCTION(FUNC_MAIN,0,false,)
INSTRUCTION(HERE_STOP,0,false,)

INSTRUCTION(RETURN,0,false,
	nextpos = li.size();
)

// Allocate on stack
INSTRUCTION(ALLOC_STACK,1,false,
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
INSTRUCTION(INT_CONST,2,true,
	co = li[aktpos]; pos = li[aktpos+1];
	INTREF(st[pos]) = co;
	ip[pos] = false;
	//ip[li[aktpos+1]+1] = false;
	//ip[li[aktpos+1]+2] = false;
	//ip[li[aktpos+1]+3] = false;
	nextpos = aktpos+2;
)

// Character constant
INSTRUCTION(CHAR_CONST,2,true,
	co = li[aktpos]; pos = li[aktpos+1];
	CHARREF(st[pos]) = co;
	ip[pos] = false;
	//ip[li[aktpos+1]+1] = false;
	//ip[li[aktpos+1]+2] = false;
	//ip[li[aktpos+1]+3] = false;
	nextpos = aktpos+2;
)

// Copy on stack
INSTRUCTION(COPY_STACK,3,true,
	posa = li[aktpos]; len = li[aktpos+1]; posb = li[aktpos+2];
	for (int i = 0; i < len; i++) {
		st[posb+i] = st[posa+i];
		ip[posb+i] = ip[posa+i];
	}
	nextpos = aktpos+3;
)

// Print integer
INSTRUCTION(PRINT_INT,1,false,
	pos = li[aktpos];
	printf("%d\n", INTREF(st[pos]));
	nextpos = aktpos+1;
)

// Print character
INSTRUCTION(PRINT_CHAR,1,false,
	pos = li[aktpos];
	printf("%c", CHARREF(st[pos]));
	nextpos = aktpos+1;
)

// Allocate fixed amount on heap and save pointer
INSTRUCTION(ALLOC_HEAP_CONSTAMOUNT,2,false,
	pos = li[aktpos]; len = li[aktpos+1];
	INTREF(st[pos]) = alloc(len, stat);
	ip[pos] = true;
	//ip[pos+1] = false;
	//ip[pos+2] = false;
	//ip[pos+3] = false;
	nextpos = aktpos+2;
)

// Allocate variable amount on heap and save pointer
INSTRUCTION(ALLOC_HEAP_VARAMOUNT,3,false,
	len = li[aktpos]; posa = li[aktpos+1]; posb = li[aktpos+2];
	INTREF(st[posb]) = alloc(len*INTREF(st[posa]), stat);
	ip[posb] = true;
	//ip[pos+1] = false;
	//ip[pos+2] = false;
	//ip[pos+3] = false;
	nextpos = aktpos+3;
)

// Set on heap
INSTRUCTION(SET_HEAP,4,false,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2]; len = li[aktpos+3];
	for (int i = 0; i < len; i++) {
		stat.hash[INTREF(st[posb])+posc+i] = st[posa+i];
		stat.hashispointer[INTREF(st[posb])+posc+i] = ip[posa+i];
	}
	nextpos = aktpos+4;
)

// Get from heap
INSTRUCTION(GET_HEAP,4,false,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2]; len = li[aktpos+3];
	for (int i = 0; i < len; i++) {
		st[posc+i] = stat.hash[INTREF(st[posa])+posb+i];
		ip[posc+i] = stat.hashispointer[INTREF(st[posa])+posb+i];
	}
	nextpos = aktpos+4;
)

// Set on heap array
INSTRUCTION(SET_ARRAY,4,false,
	len = li[aktpos]; posa = li[aktpos+1]; posb = li[aktpos+2]; posc = li[aktpos+3];
	for (int i = 0; i < len; i++) {
		stat.hash[INTREF(st[posb])+len*INTREF(st[posc])+i] = st[posa+i];
		stat.hashispointer[INTREF(st[posb])+len*INTREF(st[posc])+i] = ip[posa+i];
	}
	nextpos = aktpos+4;
)

// Get from heap array
INSTRUCTION(GET_ARRAY,4,false,
	len = li[aktpos]; posa = li[aktpos+1]; posb = li[aktpos+2]; posc = li[aktpos+3];
	for (int i = 0; i < len; i++) {
		st[posc+i] = stat.hash[INTREF(st[posa])+len*INTREF(st[posb])+i];
		ip[posc+i] = stat.hashispointer[INTREF(st[posa])+len*INTREF(st[posb])+i];
	}
	nextpos = aktpos+4;
)

// Jump if
INSTRUCTION(JUMPIF,2,false,
	posa = li[aktpos]; posb = li[aktpos+1];
	if (st[posa] == 0)
		nextpos = sto[posb];
	else
		nextpos = aktpos+2;
)

// Jump
INSTRUCTION(JUMP,1,false,
	posa = li[aktpos];
	nextpos = sto[posa];
)

// Call function
INSTRUCTION(CALL,1,false,
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
INSTRUCTION(DUMP_STACK,0,false,
	dump_stack();
	nextpos = aktpos;
)

// Dump heap
INSTRUCTION(DUMP_HEAP,0,false,
	dump_heap();
	nextpos = aktpos;
)

INSTRUCTION(PLUS,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	INTREF(st[posc]) = INTREF(st[posa])+INTREF(st[posb]);
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(MINUS,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	INTREF(st[posc]) = INTREF(st[posa])-INTREF(st[posb]);
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(TIMES,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	INTREF(st[posc]) = INTREF(st[posa])*INTREF(st[posb]);
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(DIV,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	INTREF(st[posc]) = INTREF(st[posa])/INTREF(st[posb]);
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(MOD,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	INTREF(st[posc]) = INTREF(st[posa])%INTREF(st[posb]);
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(EQUAL,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	BOOLREF(st[posc]) = INTREF(st[posa]) == INTREF(st[posb]) ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(LESS,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	BOOLREF(st[posc]) = INTREF(st[posa]) < INTREF(st[posb]) ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(GREATER,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	BOOLREF(st[posc]) = INTREF(st[posa]) > INTREF(st[posb]) ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(LESSOREQUAL,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	BOOLREF(st[posc]) = INTREF(st[posa]) <= INTREF(st[posb]) ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(GREATEROREQUAL,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	BOOLREF(st[posc]) = INTREF(st[posa]) >= INTREF(st[posb]) ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(UNEQUAL,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	BOOLREF(st[posc]) = INTREF(st[posa]) != INTREF(st[posb]) ? 1 : 0;
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(AND,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	BOOLREF(st[posc]) = (INTREF(st[posa])!=0 && INTREF(st[posb])!=0 ? 1 : 0);
	ip[posc] = false;
	nextpos = aktpos+3;
)

INSTRUCTION(OR,3,true,
	posa = li[aktpos]; posb = li[aktpos+1]; posc = li[aktpos+2];
	BOOLREF(st[posc]) = (INTREF(st[posa])!=0 || INTREF(st[posb])!=0 ? 1 : 0);
	ip[posc] = false;
	nextpos = aktpos+3;
)
