#ifndef LOCATION_H
#define LOCATION_H

struct Location {
	int first_line;
	int first_column;
	int last_line;
	int last_column;

	Location();
	Location(int fl, int fc, int ll, int lc) : first_line(fl), first_column(fc), last_line(ll), last_column(lc) {}
};

Location operator+(const Location &a, const Location &b);

#define printerr(format, ...) do { fprintf(stderr, "%d.%d-%d.%d: " format, loc.first_line, loc.first_column, loc.last_line, loc.last_column, ##__VA_ARGS__); ParseRes->haserror = true; return; } while(0)

#define printsyntaxerr(loc, format, ...) do { fprintf(stderr, "%d.%d-%d.%d: " format, loc.first_line, loc.first_column, loc.last_line, loc.last_column, ##__VA_ARGS__); ParseRes->haserror = true; } while(0)

#endif // LOCATION_H
