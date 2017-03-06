#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <iterator>
#include <math.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>

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
    if (argc != 4)
    {
        printf("usage: ./csv2colstore [csv_file] [colstore_name] [page_size]");
        exit(1);
    }

    // Setup our arguments
    const char *csv_file_name = argv[1];
    const char *colstore_name = argv[2];
    long page_size = strtol(argv[3], NULL, 10);
    int slot_size = (ATTR_NUM * ATTR_SIZE) + 99 + 2; // + 99 because CSVs will have 99 commas. + 1 for null byte +1 for new line

    // http://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
    struct stat st = {0};
    if (stat(colstore_name, &st) == -1) {
        mkdir(colstore_name, 0700);
    }

    // Open our csv & page FPs
    FILE *csv_file_ptr = fopen(csv_file_name, "r");

    // For each attribute we open a file for it.
    Heapfile **heap_files = (Heapfile **) malloc(sizeof(Heapfile) * ATTR_NUM);
    Page **heap_pages = (Page **) malloc(sizeof(Page) * ATTR_NUM);

    // Setup buffers for filename concatenation
    char *file_name_buffer = new char[1000];
    memset(file_name_buffer, '\0', 1000);

    for (int i = 0; i < ATTR_NUM; i++) {
        // Changing the int to a string
        char *buf1 = new char[ATTR_SIZE];
        memset(buf1, '\0', ATTR_SIZE);
        snprintf(buf1, ATTR_SIZE, "%d", i);

        // concat the name
        strcat(file_name_buffer, colstore_name);
        strcat(file_name_buffer, "/");
        strcat(file_name_buffer, buf1);

        FILE* file_ptr = fopen(file_name_buffer, "w+");

        // Create a new heapfile & initialize it.
        heap_files[i] = new Heapfile();
        init_heapfile(heap_files[i], page_size, file_ptr, true);

        // For each column we need to setup the page
        heap_pages[i] = buildEmptyPage(heap_files[i]);
        
        memset(file_name_buffer, '\0', 1000);
    }

    char *line = new char[slot_size];
    std::vector<std::string> attributes_in_line;

    Page *page = new Page();
    init_fixed_len_page(page, page_size, ATTR_SIZE * ATTR_NUM);
    PageID pid;
    Record* record;
    int tuple_id = 0;

    // For each line read from the CSV
    while (fgets(line, slot_size, csv_file_ptr) != NULL) {
        attributes_in_line = split(line, ',');
        tuple_id += 1;

        // For each column in the attributes_in_line, we add it to the record.
        for (int i = 0; i < attributes_in_line.size(); i++) {
            record = new Record();

            char *buf1 = new char[ATTR_SIZE];
            memset(buf1, '\0', ATTR_SIZE);
            snprintf(buf1, ATTR_SIZE, "%d", tuple_id);

            record->push_back(buf1);
            record->push_back(attributes_in_line.at(i).c_str());

            int success = add_fixed_len_page(heap_pages[i], record);

            if (success < 0) {
                pid = alloc_page(heap_files[i]);
                write_page(heap_pages[i], heap_files[i], pid);

                // Initialize new page
                heap_pages[i] = new Page();
                init_fixed_len_page(heap_pages[i], page_size, ATTR_SIZE * ATTR_NUM);

                // Add record into new page.
                add_fixed_len_page(heap_pages[i], record);
            }
        }
    }

    // Make sure to write out any additional records & close file ptrs
    for (int i = 0; i < ATTR_NUM; i++) {
        pid = alloc_page(heap_files[i]);
        write_page(heap_pages[i], heap_files[i], pid);

        fclose(heap_files[i]->file_ptr);
    }

    free(heap_files);
    free(heap_pages);

    fclose(csv_file_ptr);
    // fclose(heap_file_ptr);
}

