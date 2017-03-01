// LICENSE: hey no use for skool work

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include "library.h"

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
	char *data = (char *) page->data;
	int num_free_slots = 0;

	for (int i = 0; i < fixed_len_page_capacity(page); i++) {
		if (data[i * page->slot_size] == '\0') {
			num_free_slots += 1;
		}
	}

	return num_free_slots;
}

/**
 * Write a record into a given slot.
 */
int write_fixed_len_page(Page *page, int slot, Record *r) {
	if (fixed_len_page_freeslots(page) < 1) {
		return -1;
	}

	int slot_offset = slot * page->slot_size;
	if (((char *) page->data)[slot_offset] == '\0') {
		// create buf to read record into
		char* record_buf = (char *) malloc(fixed_len_sizeof(r));

		// null terminate buf to ensure no problems
		memset(record_buf, '\0', sizeof(char) * fixed_len_sizeof(r));
		fixed_len_write(r, record_buf);

		memcpy(((char *) page->data) + slot_offset, record_buf, fixed_len_sizeof(r));

		free(record_buf);

		return slot_offset;
	} else {
		return -1;
	}
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

		// printf("Slot#: %i, %c\n", i, ((char *) page->data)[slot_offset]);
		if (((char *) page->data)[slot_offset] == '\0') {
			printf("WRITING: %s\n", r->at(0));
			write_fixed_len_page(page, i, r);
			printf("AFTER INSERTION: %s\n", (char *) page->data);
			return slot_offset;
		}
	}

	return -1;
}
