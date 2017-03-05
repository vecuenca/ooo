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
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file, bool create_new_heap_file);
PageID alloc_page(Heapfile *heapfile);
void read_page(Heapfile *heapfile, PageID pid, Page *page);
void write_page(Page *page, Heapfile *heapfile, PageID pid);

// misc timing methods
long getTime();

// iterators
class HeapDirectoryIterator {
    public:
        HeapDirectoryIterator(Heapfile *heapfile);
        bool hasNext();
        Page *next();
    private:
        Heapfile heap;
        Page *directory_page;
};

class DirectoryPageIterator {
    public:
        DirectoryPageIterator(Heapfile *heapfile, Page *page);
        bool hasNext();
        Record *next();
    private:
        Heapfile heap;
        Page *current_page;
        RecordID *current_record;
};

class RecordIterator {
    public:
        RecordIterator(Heapfile *heap);
        Record next();
        bool hasNext();
    private:
        Heapfile heap;
        RecordID *current_record;
        Page *current_page; 
        Page *current_data_page;

        HeapDirectoryIterator *heap_dir_iter;
        DirectoryPageIterator *dir_page_iter;
};