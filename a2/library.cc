#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
typedef const char* V;
typedef std::vector<V> Record;

// We assume that there is only one table schema. 
// 100 attributes, each attribute is 10 bytes each. 
// So, records in the table are fixed length.
const int ATTR_NUM  = 100;
const int ATTR_SIZE = 10;

int fixed_len_sizeof(Record *record) {
    return ATTR_NUM * ATTR_SIZE;
}

void fixed_len_write(Record *record, void *buf) {
    
}

void fixed_len_read(void *buf, int size, Record *record) {
    assert(size == fixed_len_sizeof(record));

    for (int i = 0; i < size / sizeof(V)) {
        memcpy((char *) record->at(i), (char *) buf + sizeof(V) * i, sizeof(V));
    }
}

void init_fixed_len_page(Page *page, int page_size, int slot_size) {
	page->page_size = page_size;
	page->slot_size = slot_size;

	page->data = new char[page_size];
}

/**
 * Calculates the maximal number of records that fit in a page.
 */
int fixed_len_page_capacity(Page *page) {
	return page->page_size / (sizeof(char) + page->slot_size);
}	


/**
 * Calculate the number of free slots in the page
 */
int fixed_len_page_freeslots(Page *page) {
	char *data = page->data;
	int num_slots = 0;
	for (int i = 0; i < fixed_len_page_capacity(page); i++) {
		slots += 1 - directory[i];
	}
	return slots;
}

/**
 * Add a record to the page.
 * Returns the record slot offset if successful
 * -1 if unsuccessful (page is FULL)
 */
int add_fixed_len_page(Page *page, Record *r) {
	if (sizeof(V) 	< 1) {
		return -1;
	}




}
