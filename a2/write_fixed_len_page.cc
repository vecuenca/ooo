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
    if (argc != 4)
    {
        printf("usage: ./write_fixed_len_page [csv_file] [page_file] [page_size]");
        exit(1);
    }

    // Setup our arguments
    const char *csv_file_name = argv[1];
    const char *page_file_name = argv[2];
    long page_size = strtol(argv[3], NULL, 10);
    int slot_size = (ATTR_NUM * ATTR_SIZE) + 99 + 2; // + 99 because CSVs will have 99 commas. + 1 for null byte +1 for new line

    // Open our csv & page FPs
    FILE *csv_file_ptr = fopen(csv_file_name, "r");
    FILE *page_file_ptr = fopen(page_file_name, "w");

    char *line = new char[slot_size];
    std::vector<std::string> attributes_in_line;
    Record *record = new Record();
    Page *page = new Page();
    init_fixed_len_page(page, page_size, ATTR_SIZE * ATTR_NUM);
    int num_records = 0;
    int num_pages = 1;

    // Start timing
    long total_time_elapsed = 0;
    long write_start_time = getTime();

    // For each line read from the CSV
    while (fgets(line, slot_size, csv_file_ptr) != NULL) {
        attributes_in_line = split(line, ',');
        record = new Record();
        num_records += 1;

        // For each column in the attributes_in_line, we add it to the record.
        for (int i = 0; i < attributes_in_line.size(); i++) {
            record->push_back(attributes_in_line.at(i).c_str());
        }

        // Try to add record into new page
        int success = add_fixed_len_page(page, record);
        if (success < 0) {
            // Write out
            fwrite((char *) page->data, sizeof(char), page_size, page_file_ptr);
            
            // Initialize new page
            page = new Page();
            init_fixed_len_page(page, page_size, ATTR_SIZE * ATTR_NUM);
            num_pages += 1;

            // Add record into new page.
            add_fixed_len_page(page, record);
        }
    }
    fwrite((char *) page->data, sizeof(char), page_size, page_file_ptr);

    fclose(csv_file_ptr);
    fclose(page_file_ptr);

    total_time_elapsed = getTime();

    printf("NUMBER OF RECORDS: %i\n", num_records);
    printf("NUMBER OF PAGES: %i\n", num_pages);
    printf("Time: %ld milliseconds\n", total_time_elapsed - write_start_time);
}

