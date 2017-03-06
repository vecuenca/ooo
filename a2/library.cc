// LICENSE: hey no use for skool work

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <string>
#include <math.h>
#include <sys/timeb.h>
#include <sstream>

#include "library.h"


long getTime()
{
  struct timeb t;
  ftime(&t);
	return t.time * 1000 + t.millitm;
}

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
		record->push_back((char *) buf + ATTR_SIZE * i);
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
void write_fixed_len_page(Page *page, int slot, Record *r) {
	int slot_offset = slot * page->slot_size;

	// create buf to read record into
	char* record_buf = (char *) malloc(fixed_len_sizeof(r));

	// null terminate buf to ensure no problems
	memset(record_buf, '\0', sizeof(char) * fixed_len_sizeof(r));
	fixed_len_write(r, record_buf);

	memcpy(((char *) page->data) + slot_offset, record_buf, fixed_len_sizeof(r));

	free(record_buf);
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

/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r) {
	int slot_offset = slot * page->slot_size;

	fixed_len_read((char *)(page->data) + slot_offset, fixed_len_sizeof(r), r);
}

// --------------------
// HEAP-Related methods
// --------------------

// Helpers, helps us keep the actual methods easier to read.
Page* buildEmptyPage(Heapfile *heapfile) {
	Page *page = new Page();

	init_fixed_len_page(page, heapfile->page_size, ATTR_NUM * ATTR_SIZE);

	return page;
}

const char* LAST_DIRECTORY = "LAST_DIR__";
const char* HAS_NEXT_DIRECTORY = "HAS_NEXT__";

// Each directory page's first row's first attribute indicates whether there's a next directory page.
// Enables us to keep track of our directory pages as a linked list.
Record* buildDirectoryMetadata() {
	Record *record = new Record();
	record->push_back(LAST_DIRECTORY); // Defaulting our first attribute to be the end of the linkedlist

	return record;
}

Record* buildDirectoryEntry(int page_offset, int free_space) {
	Record *record = new Record();

	char *buf1 = new char[ATTR_SIZE];
	char *buf2 = new char[ATTR_SIZE];

	memset(buf1, '\0', ATTR_SIZE);
	memset(buf2, '\0', ATTR_SIZE);

	snprintf(buf1, ATTR_SIZE, "%d", page_offset);
	snprintf(buf2, ATTR_SIZE, "%d", free_space);

	// printf("page_offset: %s, free_space: %s",  buf1, buf2);
	record->push_back(buf1);
	record->push_back(buf2);

	return record;
}

Page* buildDirectory(Heapfile* heapfile) {
	Page *page = buildEmptyPage(heapfile);
	Record *directory_entry = buildDirectoryMetadata();

	// Add metadata to first slot.
	add_fixed_len_page(page, directory_entry);

	return page;
}

void getLastDirectory(Heapfile *heapfile, Page* last_directory_page, int *number_of_directory_entries) {
	Record *page_first_record;
	
	fseek(heapfile->file_ptr, 0, SEEK_SET);
	int i = 0;

	do {
		page_first_record = new Record();

		// Read directory page
		fread(last_directory_page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);
		
		// Read first record of directory page (stores the offset for next Directory, 0 otherwise)
		fixed_len_read(last_directory_page->data, ATTR_NUM * ATTR_SIZE, page_first_record);

		// Count the number of entries in current directory, we do capacity - freeslots - 1 because every heap directory's first record stores heap metadata.
		*number_of_directory_entries = *number_of_directory_entries + (fixed_len_page_capacity(last_directory_page) - fixed_len_page_freeslots(last_directory_page) - 1);

		printf("\n[Getting Last Directory] %i\n", ++i);
		printf("%.*s\n", ATTR_SIZE, page_first_record->at(0));

	} while(strncmp(page_first_record->at(0), LAST_DIRECTORY, ATTR_SIZE) != 0);
	printf("[Last Directory] %i\n", i);

	// Because of fread, our pointer will point to the next page. so we revert it back to
	// one page before.
	fseek(heapfile->file_ptr, heapfile->page_size * -1, SEEK_CUR);
}


/**
 * Initalize a heapfile to use the file and page size given.
 * We don't know if the file exists or not, so we just pass a bool flag for simplicity.
 * Assumptions: read & write access. 
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file, bool create_new_heap_file) {
	heapfile->file_ptr = file;
	heapfile->page_size = page_size;

	if (create_new_heap_file) {
		Page* first_page = buildDirectory(heapfile);

		fwrite(first_page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);

		rewind(file);
	}
};

// Assuming the next page is a data page.
void _shuffle_data_pages_down(Heapfile *heapfile) {

	int page_size = heapfile->page_size;
	int slot_size = ATTR_NUM * ATTR_SIZE;

	Page *page = new Page();
	init_fixed_len_page(page, page_size, slot_size);

	Page *prev_page = new Page();
	init_fixed_len_page(prev_page, page_size, slot_size);

	// If we try to read a page and it's empty we go back to where we started
	// fread(page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);
	// fread()
	if (fread(prev_page->data, sizeof(char), page_size, heapfile->file_ptr) == 0) {
		fseek(heapfile->file_ptr, 0, SEEK_END);
	} else {
		int data_pages_read = 1;

		// Traverse to the end of the file
		// change to fread
		while (fread(page->data, sizeof(char), page_size, heapfile->file_ptr) > 0) {
			data_pages_read += 1;
			// Pointer is at end of current page, we need to go back to start of current page
			fseek(heapfile->file_ptr, page_size * -1, SEEK_CUR);

			// Write previous page data to current page
			fwrite((char *) prev_page->data, sizeof(char), page_size, heapfile->file_ptr);

			// Copy current page data into prev.
			memcpy(prev_page->data, page->data, page_size);

			// reinitialize current page.
			init_fixed_len_page(page, page_size, slot_size);
		}

		if (data_pages_read == 1) {
			fseek(heapfile->file_ptr, 0, SEEK_END);
		}

		// Left with one more to write
		fwrite((char *) prev_page->data, sizeof(char), page_size, heapfile->file_ptr);

		// Go back data_pages_read + 1 data_page, so we end off at where we started.
		fseek(heapfile->file_ptr, -1 * data_pages_read * page_size + -1* page_size, SEEK_CUR);
	}
}

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 *
 * Step 1: Add a new entry to the last heap directory page. 
 * (If it's full, we need to create a new heap directory page)
 * 
 * Step 2: Add a new empty page & append it.
 */
PageID alloc_page(Heapfile *heapfile) {
	Page *last_directory_page = buildEmptyPage(heapfile);
	int number_of_directory_entries = 0;

	// Step 1.1 Get the last directory & make sure there's space to add a new entry
	getLastDirectory(heapfile, last_directory_page, &number_of_directory_entries);

	if (fixed_len_page_freeslots(last_directory_page) == 0) {
		// Construct new record for the metadata that indicates there's a new directory.
		Record *last_directory_page_metadata = new Record();
		last_directory_page_metadata->push_back(HAS_NEXT_DIRECTORY);

		// Update current last_directory_page point to indicate there's a next directory.
		write_fixed_len_page(last_directory_page, 0, last_directory_page_metadata);

		// Write last_directory_page to disk.
		fwrite(last_directory_page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);

		_shuffle_data_pages_down(heapfile);

		// Create the last_directory_page & write it to disk.
		last_directory_page = buildDirectory(heapfile);
		fwrite(last_directory_page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);

		// Writing advances our pointer, we move it back to the start of the page
		// that we just wrote.
		fseek(heapfile->file_ptr, -1 * heapfile->page_size, SEEK_CUR);
	}

	// Step 1.2 Add a new directory entry for our newly allocated page!
	Record *heap_directory_entry = buildDirectoryEntry(number_of_directory_entries, fixed_len_page_freeslots(last_directory_page));
	add_fixed_len_page(last_directory_page, heap_directory_entry);

	// Step 1.3 Write the entry out to the last directory entry.
	fwrite(last_directory_page->data, sizeof(char), last_directory_page->page_size, heapfile->file_ptr);
	FILE *temp = fopen((char *) heap_directory_entry->at(0), "w");
	fwrite(last_directory_page->data, sizeof(char), last_directory_page->page_size, temp);
	fclose(temp);
	
	// Step 2: append our newly allocated data page to the last data page.
	// Seek to the end of the file = last data page
	Page *data_page = buildEmptyPage(heapfile);
	fseek(heapfile->file_ptr, 0L, SEEK_END);

	// Write our newly allocated page to disk.
	fwrite((char *) data_page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);

	// rewind our file-pointer, since it's currently at the end.
	rewind(heapfile->file_ptr);

	return number_of_directory_entries;
}

// THIS SHIT (BELOW) AINT DONE YET.

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page) {
	Page *last_directory_page = buildEmptyPage(heapfile);
	int number_of_directory_entries = 0;

	rewind(heapfile->file_ptr);

	// We want to advance our file pointer to the last directory
	getLastDirectory(heapfile, last_directory_page, &number_of_directory_entries);
	// Seek to the pageId
	fseek(heapfile->file_ptr, (pid * heapfile->page_size) + heapfile->page_size, SEEK_CUR);

	// Initialize a new page with nulls
	page = buildEmptyPage(heapfile);

	// Read the page
	fread(page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);

	// rewind our file-pointer, since it's currently at the end.
	rewind(heapfile->file_ptr);
}

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid) {
	Page *last_directory_page = buildEmptyPage(heapfile);
	int number_of_directory_entries = 0;

	rewind(heapfile->file_ptr);
	
	// We want to advance our file pointer to the last directory
	getLastDirectory(heapfile, last_directory_page, &number_of_directory_entries);

	int directory_capacity = fixed_len_page_capacity(last_directory_page) - 1; // Because metadata takes 1 record
	double directory_pages_used = ceil(number_of_directory_entries / (double) directory_capacity);

	int offset = (pid * heapfile->page_size) + (directory_pages_used * heapfile->page_size);

	// Seek to the pageId
	fseek(heapfile->file_ptr, offset, SEEK_SET);

	// write the page
	fwrite((char *) page->data, sizeof(char), page->page_size, heapfile->file_ptr);

	// // rewind our file-pointer, since it's currently at the end.
	rewind(heapfile->file_ptr);	

	// Now we need to update our directory record's free space attribute
	// int directory_page_offset = floor(pid / (double) directory_capacity) * heapfile->page_size;
	// double directory_entry_offset = remainder((double) pid, (double) directory_capacity) + 1;

	// fseek(heapfile->file_ptr, directory_page_offset, SEEK_SET);

	// Page* dir_page = buildEmptyPage(heapfile);
	// fread(dir_page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);

	// Record* entry = new Record();
	// char *buf1 = new char[ATTR_SIZE];
	// char *buf2 = new char[ATTR_SIZE];

	// memset(buf1, '\0', ATTR_SIZE);
	// memset(buf2, '\0', ATTR_SIZE);

	// snprintf(buf1, ATTR_SIZE, "%d", pid);
	// snprintf(buf2, ATTR_SIZE, "%d", fixed_len_page_freeslots(page));
	// entry->push_back(buf1);
	// entry->push_back(buf2);

	// write_fixed_len_page(dir_page, directory_entry_offset, entry);

	// fseek(heapfile->file_ptr, -1 * heapfile->page_size, SEEK_CUR);

	// fwrite((char *) dir_page->data, sizeof(char), page->page_size, heapfile->file_ptr);

	// rewind(heapfile->file_ptr);
}

class PageIterator {
	public:
		PageIterator(Page *page);
		Record next();
		bool hasNext();
	private:
		Page *page;
		int current_slot;
};

// PageIterator::PageIterator(Page *page) {
// 	page = page;
// 	current_slot = 0;
// }

// Record PageIterator::next() {
// 	Record *temp_record = new Record();
	
// 	fixed_len_read((char *) page->data + current_slot * page->slot_size, page->slot_size, temp_record);

// 	current_slot += 1;

// 	return temp_record;
// }


// // Assuming that there are no holes.
// bool PageIterator::hasNext() {
// 	// Check if we've exceeded the capacity
// 	if (current_slot >= page->page_size / page->slot_size) {
// 		return false;
// 	}

// 	int slot_offset = current_slot * page->slot_size;
	
// 	// Attempt to read the next slot.
// 	if (((char* ) page->data)[slot_offset] == '\0') {
// 		return false;
// 	} else {
// 		return true;
// 	}
// }

// Iterator for each directory
HeapDirectoryIterator::HeapDirectoryIterator(Heapfile *heap) {
	heap = heap;
	directory_page = new Page;

	// load first directory page into directory page
	init_fixed_len_page(directory_page, heap->page_size, ATTR_NUM * ATTR_SIZE);
}

Page *HeapDirectoryIterator::next() {
	// sanity check
	assert(hasNext());	

	// advance file ptr to next dir page
	fseek(heap.file_ptr, heap.page_size, SEEK_CUR);

	// read current ptr pos into directory page
	fread(directory_page->data, heap.page_size, 1, heap.file_ptr);

	return directory_page;
}

bool HeapDirectoryIterator::hasNext() {
	// read first record current dir page
	Record *dir_record;
	fixed_len_read(directory_page->data, ATTR_NUM * ATTR_SIZE, dir_record);

	// check the 0th attr of the record vector
	return dir_record->at(0) == HAS_NEXT_DIRECTORY;
}

// Iterator over records for page in a single directory_page
DirectoryPageIterator::DirectoryPageIterator(Heapfile *heap, Page *page) {
	heap = heap;

	// what is pageid used for?
	current_record          = new RecordID();
	current_record->page_id = 0;
	current_record->slot    = 0;
}

bool DirectoryPageIterator::hasNext() {
	// check if rest of page has available slot with a record
	return current_record->slot <= fixed_len_page_capacity(current_page);
}

Record *DirectoryPageIterator::next() {
	// assume there exists a next page
	Record *next_record;
	char *data = (char *) current_page->data;
	// loop through slots for next record, and return it
	for (int i = current_record->slot; i < fixed_len_page_capacity(current_page); i++) {
		int slot_offset = i * current_page->slot_size;
		// is there a record at this slot offset?
		if (data[slot_offset] != '\0') {
			fixed_len_read((char *) data + slot_offset, ATTR_NUM * ATTR_SIZE, next_record);
			//set current record slot 
			current_record->slot = i;
			return next_record;
		}
	}
}

// Iterator over records in a data page
RecordIterator::RecordIterator(Heapfile *heap) {
		// record to keep track of where we are
		current_record           = new RecordID();
		current_record->page_id  = 0;
		current_record->slot     = 0;

		heap = heap;

		// initialize directory iterator
		heap_dir_iter = new HeapDirectoryIterator(heap);

		// load in first directory page to directory page iterator
		Page *directory_page;
		read_page(heap, current_record->page_id, directory_page);
		dir_page_iter = new DirectoryPageIterator(heap, directory_page);
}

// assume there is another record to find
Record RecordIterator::next() {
	// record to return
	Record* next_record;
	char *data = (char *) current_page->data;
	for (int i = current_record->slot; i < fixed_len_page_capacity(current_page); i++) {
		int slot_offset = i * current_page->slot_size;
		if (data[slot_offset] != '\0') {
			fixed_len_read((char *) data + slot_offset, ATTR_NUM * ATTR_SIZE, next_record);
			//set current record slot 
			current_record->slot = i;
			break;
		}
	}
	// find next record in current page	
	if (current_record->slot >= fixed_len_page_capacity(current_page)) {
		if (dir_page_iter->hasNext()) {
			// current_data_page = dir_page_iter->next();
		} else if (heap_dir_iter->hasNext()) {
			// TODO: get next directory, update curr page to first page of that directory
		}
	}
	return *next_record;
}

bool RecordIterator::hasNext() {
	// check if:
	//	there are more slots in this page
	//	if there is another page in this directory
	//  there is another directory`
	return (current_record->slot <= fixed_len_page_capacity(current_page)
	 && !dir_page_iter->hasNext() && !heap_dir_iter->hasNext());
}

// RecordIterator::RecordIterator(Heapfile *heapfile) {
// 	heap = heapfile;

// 	// Initialize our RecordId
// 	current_record = new RecordID();
// 	current_record->page_id = 0;
// 	current_record->slot = 0;

// 	// Read page from disk and initialize our page iterator
// 	Page *page;
// 	read_page(heap, current_record->page_id, page);
// 	page_iterator = new PageIterator(page);
// }

// bool RecordIterator::_hasNextRecordInCurrentPage() {
// 	return page_iterator->hasNext();
// }

// bool RecordIterator::_hasNextPage() {
// 	Page* page;
// 	read_page(heap, current_record->page_id + 1, page);
// }

// bool RecordIteartor::_hasRecordInNextPage() {

// }

// Record RecordIterator::next() {
// 	Page* page;
// 	Record* record;
// 	int record_size = fixed_len_sizeof(record);

// 	page = page_iterator->next();

// 	// Read record from given slot.
// 	// Note: First time using thie method, might be wrong..
// 	fixed_len_read((char *) page->data + current_record->slot * record_size, record_size, record);

// 	// TODO: increment recordId if there's a next recordId. set _hasNext = true
// 	// otherwise check nextPage and if there's a record, increment pageId and reset recordId. Set _hasNext = true
// 	// if there's no next page set _hasNext = false


// 	return *record;
// }

// bool RecordIterator::hasNext() {
// 	if (_hasNextRecordInCurrentPage() || 
// 		(_hasNextPage() && _hasRecordInNextPage())) {
// 		return true;
// 	} else {
// 		return false;
// 	}
// }