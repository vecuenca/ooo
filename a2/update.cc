#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <iterator>
#include <math.h>

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

int main(int argc, char *argv[]) {
    if (argc != 6) {
		printf("usage: ./update [heapfile] [record_id] [attribute_id] [new_value] [page_size]\n");
    	exit(1);
	}
    long slot_size = ATTR_NUM * ATTR_SIZE;

    // fetch args
	char *heapfile_name = argv[1];
    char *record_id     = argv[2];
    long       attr_id       = strtol(argv[3], NULL, 10);
    const char       *new_value     = argv[4];
	long       page_size     = strtol(argv[5], NULL, 10);

    // record id is in format [page_id],[slot_offset]
    std::vector<std::string> record_vector;
    record_vector    = split(record_id, ',');
    long int page_id     = strtol(record_vector.at(0).c_str(), NULL, 10);
    long int slot_offset = strtol(record_vector.at(1).c_str(), NULL, 10);
    
    // open up heap 
   	FILE *heap_file_ptr = fopen(heapfile_name, "r+");

    // initialize heap struct
    Heapfile *heapfile = new Heapfile();
	init_heapfile(heapfile, page_size, heap_file_ptr, false);

    // navigate to page
    int num_dir_pages = 0;
    Page *page = buildEmptyPage(heapfile);

    // seek to last directory 
	getLastDirectory(heapfile, page, &num_dir_pages);

	// seek to page where record is stored
    // page_id + 1 because we seek to the last directory page
	fseek(heapfile->file_ptr, heapfile->page_size * (page_id + 1) ,SEEK_CUR);

    // Read the page into memory.
    fread(page->data, sizeof(char), heapfile->page_size, heapfile->file_ptr);

    // Read the record.
    Record *record = new Record();
    fixed_len_read(((char *) page->data) + slot_size * slot_offset, ATTR_NUM * ATTR_SIZE, record);

    // Construct the updated record, ensuring to replace the attribute
    Record *updated_record = new Record();
    for (int i = 0; i < ATTR_NUM; i++) {
        if (i != attr_id) {
            updated_record->push_back(record->at(i));
        } else {
            char* new_val = new char[ATTR_SIZE];
            memset(new_val, '\0', ATTR_SIZE);
            strncpy(new_val, new_value, ATTR_SIZE);
            
            updated_record->push_back(new_val);
        }
    }

    // Save our updated record at the given slot and write out to disk.
    write_fixed_len_page(page, slot_offset, updated_record);
    write_page(page, heapfile, page_id);

    fclose(heapfile->file_ptr);

    return 0;
}
