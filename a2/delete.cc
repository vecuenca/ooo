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
    if (argc != 3) {
		printf("usage: ./delete [heapfile] [record_id] [page_size]\n");
    	exit(1);
	}
    long slot_size = ATTR_NUM * ATTR_SIZE;

    // fetch args
	char *heapfile_name = argv[1];
    char *record_id = argv[2];
    long attr_id = strtol(argv[3], NULL, 10);
    const char *new_value = argv[4];
	long page_size = strtol(argv[5], NULL, 10);

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

    // seek to record offset, overwrite
    fseek(heapfile->file_ptr, slot_size * slot_offset, SEEK_CUR);
    for (int i = 0; i < ATTR_SIZE * ATTR_NUM; i++) {
        fwrite('\0', sizeof(char), 1, heapfile->file_ptr);
    }

    fclose(heapfile->file_ptr);

    return 0;
}
