#include "parseresult.h"

ParseResult::ParseResult() {
	voidType = new PrimitiveType(0);
	intType = new PrimitiveType(1);
	boolType = new PrimitiveType(1);
	types["void"] = voidType;
	types["int"] = intType;
	types["bool"] = boolType;
}


ParseResult *global = new ParseResult();

ParseResult* ParseRes {
	return global;
}
