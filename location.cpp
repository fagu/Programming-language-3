
#include "location.h"

Location::Location() {
	first_line = -1;
	first_column = -1;
	last_line = -1;
	last_column = -1;
}

Location operator+(const Location &a, const Location &b) {
	return Location(a.first_line, a.first_column, b.last_line, b.last_column);
}
