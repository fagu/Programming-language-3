
#ifndef GARBAGECOLLECTOR_H
#define GARBAGECOLLECTOR_H

struct state;

void free(int i, state &st);
int alloc(int len, state& st);
void gc(state & st);

#endif // GARBAGECOLLECTOR_H
