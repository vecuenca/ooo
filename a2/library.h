#ifndef LIBRARY_H
#define LIBRARY_H
#endif

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
typedef const char* V;
typedef std::vector<V> Record;

typedef struct {
    void *data;
    int page_size;
    int slot_size;
} Page;

void init_fixed_len_page(Page *page, int page_size, int slot_size);
int add_fixed_len_page(Page *page, Record *r);

int fixed_len_sizeof(Record *record);

void fixed_len_write(Record *record, void *buf);