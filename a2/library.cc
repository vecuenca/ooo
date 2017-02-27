// LICENSE: hey no use for skool work

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

/**
 * Compute number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record) {
    return ATTR_NUM * ATTR_SIZE;
}

/**
 * Serialize the record to a byte array
 */
void fixed_len_write(Record *record, void *buf) {
	for(Record::iterator i = record->begin(); i != record->end(); i++) {
        memcpy(buf, *i, ATTR_SIZE);
        buf = (char *) buf + ATTR_SIZE;
	}
}

/**
 * Deserializes 'size' bytes from buf, stores in record
 */
void fixed_len_read(void *buf, int size, Record *record) {
    assert(size == fixed_len_sizeof(record));

    for (int i = 0; i < ATTR_NUM; i++) {
        memcpy((char *) record->at(i), (char *) buf + ATTR_SIZE * i, ATTR_SIZE);
    }
}

void init_fixed_len_page(Page *page, int page_size, int slot_size) {
	page->page_size = page_size;
	page->slot_size = slot_size;

	page->data = new char[page_size];
	memset(page->data, '\0', sizeof(char) * page_size);
}

/**
 * Calculates the maximal number of records that fit in a page.
 */
int fixed_len_page_capacity(Page *page) {
	return page->page_size / page->slot_size;
}	


/**
 * Calculate the number of free slots in the page
 */
int fixed_len_page_freeslots(Page *page) {
	char *data = page->data;
	int num_free_slots = 0;
	for (int i = 0; i < fixed_len_page_capacity(page); i++) {
		if (data[i * page->slot_size] == '\0') {
			num_free_slots += 1;
		}
	}
	return num_free_slots;
}

/**
 * Add a record to the page.
 * Returns the record slot offset if successful
 * -1 if unsuccessful (page is FULL)
 */
int add_fixed_len_page(Page *page, Record *r) {
	if (fixed_len_page_freeslots(page) < 1) {
		return -1;
	}

	// find first available slot?
	for (int i = 0; i < fixed_len_page_capacity(page); i++) {
		int slot_offset = i * page->slot_size;
		write_fixed_len_page(page, i, r)
	}
	return -1;
}

/**
 * Write a record into a given slot.
 */
int write_fixed_len_page(Page *page, int slot, Record *r) {
	if (fixed_len_page_freeslots(page) < 1) {
		return -1;
	}

	int slot_offset = slot * page->slot_size;
	if (data[slot_offset] == '\0') {
		// create buf to read record into
		char[] record_buf = new char[fixed_len_sizeof(r)];

		// null terminate buf to ensure no problems
		memset(record_buf, '\0', sizeof(char) * fixed_len_sizeof(r));
		fixed_len_write(r, record_buf);
		memcpy(page->data, record_buf, fixed_len_sizeof(r));

		return slot_offset;
	} else {
		return -1;
	}
}

int write_to_
