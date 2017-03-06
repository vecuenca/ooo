#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char *argv[]) {
	if (argc != 6) {
		printf("usage: ./select [heapfile] [attribute_id] [start] [end] [page_size]");
    	exit(1);
	}

	// grab args
	const char *heapfile_name   = argv[1];
	long        attribute_id    = strtol(argv[2], NULL, 10);
    const char *start           = argv[3];
    const char *end             = argv[4];
    long        page_size       = strtol(argv[5], NULL, 10);

    int slot_size = ATTR_NUM * ATTR_SIZE;

	// open up heap to scan
	FILE *heap_file_ptr = fopen(heapfile_name, "r");

    if (!heap_file_ptr) {
		printf("heapfile does not exist. Terminating.\n");
		return 0;
	}

	// initialize a heapfile struct
	Heapfile *heap   = new Heapfile();
	init_heapfile(heap, page_size, heap_file_ptr, false);

	// go past all directories
	int num_dir_pages = 0;
	Page *page = buildEmptyPage(heap);

	// seek to last directory 
	getLastDirectory(heap, page, &num_dir_pages);

	// seek to first data page
	fseek(heap->file_ptr, heap->page_size ,SEEK_CUR);

    // Setup Record
    Record* record = new Record();

	// read in the entire page
	while(fread(page->data, sizeof(char), heap->page_size, heap->file_ptr)) {
		// iterate through records
		for (int j = 0; j < fixed_len_page_capacity(page); j++) {
			record = new Record();

            // deserialize row into a record
            fixed_len_read(((char* ) page->data) + j * slot_size, slot_size, record);
            
            char* attribute_value = new char[ATTR_SIZE];
            strncpy(attribute_value, (char *) record->at(attribute_id), ATTR_SIZE);

            bool is_bigger_than_start = strncmp(attribute_value, start, ATTR_SIZE) >= 0;
            bool is_smaller_than_end = strncmp(attribute_value, end, ATTR_SIZE) <= 0;

            if (is_bigger_than_start && is_smaller_than_end) {
                printf("%.*s\n", 5, attribute_value);            
            }
		}
	}
}