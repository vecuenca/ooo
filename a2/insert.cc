#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>

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


int main(int argc, const char *argv[]) {
    if (argc != 4) {
        printf("usage: ./insert [heapfile] [csv_file] [page_size]");
        exit(1);
    }

    // grab args
    const char *heapfile_name = argv[1];
    const char *csv_file_name = argv[2];
    long       page_size      = strtol(argv[3], NULL, 10);
    int        slot_size      = (ATTR_NUM * ATTR_SIZE) + 99 + 2;

    // open up csv to scan
    FILE *csv_file_ptr = fopen(csv_file_name, "r");
    char *line = new char[slot_size];

    // open up heapfile
    FILE *heap_file_ptr = fopen(heapfile_name, "r");

    // initialize heapfile struct
    Heapfile *heapfile = new Heapfile();
    heapfile->file_ptr    = heap_file_ptr;
    heapfile->page_size   = page_size;
    
    // setup record for reading
    Record *record = new Record();
    std::vector<std::string> attributes_in_line;

    // setup page to be added to heapfile
    Page *page = new Page();

    // read csv line by line, creating a record for each line
    while (fgets(line, slot_size, csv_file_ptr) != NULL) {
        // create record
        attributes_in_line = split(line, ',');
        record = new Record();

        for (int i = 0; i < attributes_in_line.size(); i++) {
            record->push_back(attributes_in_line.at(i).c_str());
        }

        // try and add record to new page
        int success = add_fixed_len_page(page, record);
        // page is full
        if (success < 0) {
            // allocate a page and write it 
            PageID pid = alloc_page(heapfile);
            write_page(page, heapfile, pid);

            // initialize new page, add record into page
            page = new Page();
            init_fixed_len_page(page, page_size, ATTR_SIZE * ATTR_NUM);
            add_fixed_len_page(page, record);
        }
    }
    PageID pid = alloc_page(heapfile);
    write_page(page, heapfile, pid);

    fclose(heap_file_ptr);
    fclose(csv_file_ptr);

    return 0;
}