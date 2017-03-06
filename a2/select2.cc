#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <iterator>
#include <math.h>
#include <sys/timeb.h>

#include "library.h"

// http://stackoverflow.com/questions/236129/split-a-string-in-c
template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

// http://stackoverflow.com/questions/236129/split-a-string-in-c
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

int main(int argc, const char *argv[])
{
    if (argc != 6)
    {
        printf("usage: ./select2 [colstore_name] [attribute_id] [start] [end] [page_size]");
        exit(1);
    }

    // Setup our arguments
    const char *colstore_name   = argv[1];
	const char *attribute_id    = argv[2];
    const char *start           = argv[3];
    const char *end             = argv[4];
    long        page_size       = strtol(argv[5], NULL, 10);
    int slot_size = ATTR_NUM * ATTR_SIZE;

    // Construct our buffer
    char *file_name_buffer = new char[1000];
    memset(file_name_buffer, '\0', 1000);
    strcat(file_name_buffer, colstore_name);
    strcat(file_name_buffer, "/");
    strcat(file_name_buffer, attribute_id);

    // Open the column file
    FILE *col_file_ptr = fopen(file_name_buffer, "r");

    if (!col_file_ptr) {
		printf("heapfile does not exist. Terminating.\n");
		return 0;
	}

    // Initialize the heap struct
    Heapfile *heap = new Heapfile();
    init_heapfile(heap, page_size, col_file_ptr, false);

    // go past all directories
	int num_dir_pages = 0;
	Page *page = buildEmptyPage(heap);

	// seek to last directory 
	getLastDirectory(heap, page, &num_dir_pages);

	// seek to first data page
	fseek(heap->file_ptr, heap->page_size ,SEEK_CUR);

    // Setup Record
    Record* record;

    // read in the entire page
	while(fread(page->data, sizeof(char), heap->page_size, heap->file_ptr)) {
        // iterator through records
        for (int j = 0; j < fixed_len_page_capacity(page); j++) {
            record = new Record();

            // deserialize row into a record
            fixed_len_read(((char* ) page->data) + j * slot_size, slot_size, record);
            
            char* attribute_value = new char[ATTR_SIZE];
            strncpy(attribute_value, ((char* ) page->data) + j * slot_size + ATTR_SIZE, ATTR_SIZE);

            bool is_bigger_than_start = strncmp(attribute_value, start, ATTR_SIZE) >= 0;
            bool is_smaller_than_end = strncmp(attribute_value, end, ATTR_SIZE) <= 0;

            if (is_bigger_than_start && is_smaller_than_end) {
                printf("%.*s\n", 5, attribute_value);            
            }
        }
    }

    fclose(col_file_ptr);
}

