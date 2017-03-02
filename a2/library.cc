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

		if (((char *) page->data)[slot_offset] == '\0') {
			write_fixed_len_page(page, i, r);

			return slot_offset;
		}
	}

	return -1;
}

// --------------------
// HEAP-Related methods
// --------------------

/**
 * Initalize a heapfile to use the file and page size given.
 * Assumptions: read & write access. 
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file) {
	heapfile->file_ptr = file;
	heapfile->page_size = page_size;
};

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile) {
	// Initialize a new page with nulls
	Page *page = new Page();
	init_fixed_len_page(page, heapfile->page_size, ATTR_NUM * ATTR_SIZE);

	// Seek to the end of the file
	fseek(heapfile->file_ptr, 0L, SEEK_END);
	int heap_file_size = ftell(heapfile->file_ptr);

	// Write your new page out
	fwrite((char *) page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);

	// rewind our file-pointer, since it's currently at the end.
	rewind(heapfile->file_ptr);

	return heap_file_size / heapfile->page_size;
}

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page) {
	// Initialize a new page with nulls
	page = new Page();
	init_fixed_len_page(page, heapfile->page_size, ATTR_NUM * ATTR_SIZE);

	// Seek to the pageId
	fseek(heapfile->file_ptr, 0L, pid * heapfile->page_size);

	// Read the page
	fread(page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);

	// rewind our file-pointer, since it's currently at the end.
	rewind(heapfile->file_ptr);
}

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid) {
	char* data = (char *) page->data;

	// Seek to the pageId
	fseek(heapfile->file_ptr, 0L, pid * heapfile->page_size);

	// write the page
	fwrite(data, sizeof(char), heapfile->page_size, heapfile->file_ptr);

	// rewind our file-pointer, since it's currently at the end.
	rewind(heapfile->file_ptr);
}

// whoops iterator should be defined like this https://www.tutorialspoint.com/cplusplus/cpp_constructor_destructor.htm
// class RecordIterator {
//     public:
//     RecordIterator(Heapfile *heapfile) {
// 		Heapfile *heap = heapfile;

// 		// Initialize our RecordId
// 		RecordID* current_record = new RecordID();

// 		current_record->page_id = 0;
// 		current_record->slot = 0;
// 	}

// 	// TODO: this is incomplete
//     Record next() {
// 		Page* page;
// 		Record* record;
// 		int record_size = fixed_len_sizeof(record);

// 		// Get appropriate page.
// 		read_page(heap, current_record->page_id, page);

// 		// Read record from given slot.
// 		// Note: First time using thie method, might be wrong..
// 		fixed_len_read((char *) page->data + current_record->slot * record_size, record_size, record);

// 		// TODO: 
// 		// Check if there's anymore records to be read from this page
// 		// Otherwise we reset slot to 0 and increment page_id

// 		return record;
// 	}

// 	// TODO: this is incomplete
//     bool hasNext() {
// 		// If there's anymore records to be read from the page
// 			// return true
// 		// Else: Check if there are records to be read from next page (also increment our page_id)
// 			// Increment page_id & read new page
// 			// If there are records to be read from current page, return true
// 			// Else return false because pages are continguous => we shouldn't have 2 empty pages in a row.
// 	}
// };