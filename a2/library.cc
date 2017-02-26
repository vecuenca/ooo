#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
typedef const char* V;
typedef std::vector<V> Record;

int fixed_len_sizeof(Record *record) {
    return sizeof(V) * record->size();
}

void fixed_len_write(Record *record, void *buf) {
    for (Record::iterator i = record->begin(); i != record->end(); i++) {
        memcpy(buf, *i, sizeof(V));
        buf = (char *) buf + sizeof(V);
    }
}
