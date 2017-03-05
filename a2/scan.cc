#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char *argv[]) {
	if (argc != 3) {
		printf("usage: ./scan [heapfile] [page_size]");
    exit(1);
	}

	// grab args
	const char *heapfile_name = argv[1];
	long       *page_size     = strtol(argv[2], NULL, 10);

	// open up heap to scan
	FILE *heap_file_ptr = fopen(heapfile_name, "r+");

	// initialize a heapfile struct
	Heapfile heap   = new Heap();
	heap->file_ptr  = heap_file_ptr;
	heap->page_size = page_size;

	// go past all directories
	int num_dir_pages = 0;
	Page *page = buildEmptyPage(heap);

	// seek to last directory 
	getLastDirectory(heap, page, num_dir_pages);

	// seek to first data page
	fseek(heapfile->file_ptr, heap->page_size ,SEEK_CUR);

	// read in the entire page
	while(fread(page->data, sizeof(char), heap->page_size, heap->file_ptr)) {
		// iterate through records
		for (int j = 0; j < fixed_len_page_capacity(page); j++) {
			// iterate thru attributes
			for (int i = 0; i < ATTR_NUM; i++) {
				printf("%.*s", ATTR_SIZE, (char *) page->data + j * ATTR_NUM * SIZE + i * ATTR_SIZE);
				if (i != ATTR_NUM - 1) printf(",");
			}
			printf("\n");
		}
	}
}