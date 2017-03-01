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

typedef struct {
    FILE *file_ptr;
    int page_size;
} Heapfile;

typedef int PageID;
 
typedef struct {
    int page_id;
    int slot;
} RecordID;

static int ATTR_NUM  = 100;
static int ATTR_SIZE = 10;

// Page related methods
void init_fixed_len_page(Page *page, int page_size, int slot_size);
int add_fixed_len_page(Page *page, Record *r);
int fixed_len_sizeof(Record *record);
void fixed_len_write(Record *record, void *buf);

// Heap file related methods
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file);
PageID alloc_page(Heapfile *heapfile);
void read_page(Heapfile *heapfile, PageID pid, Page *page);
void write_page(Page *page, Heapfile *heapfile, PageID pid);