#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <iterator>

#include "library.h"

const int ATTR_NUM  = 100;
const int ATTR_SIZE = 10;

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

    FILE *csv_file_ptr = fopen(csv_file_name, "r");
    FILE *page_file_ptr = fopen(page_file_name, "w");

    char *line = new char[ATTR_NUM * ATTR_SIZE];
    std::vector<std::string> row;
    Record *record = new Record();
    Page *page = new Page();
    init_fixed_len_page(page, page_size, ATTR_SIZE * ATTR_NUM);
    while (fgets(line, ATTR_NUM * ATTR_SIZE, csv_file_ptr) != NULL) {
        row = split(line, ',');
        
        record = new Record();

        // For each column in the row, we add it to the record.
        for (int i = 0; i < row.size(); i++) {
            record->push_back(row.at(i).c_str());
        }

        int success = add_fixed_len_page(page, record);
        printf("Success: %d\n", success);
        // if (!success) {
        //     printf("FAILLLLLLLLed");
        // }
        // if (!success) {
        //     // Page is full, dump to page_file.
        //     fwrite((char *) page->data, 1, ATTR_SIZE * ATTR_NUM, page_file_ptr);
        //     fflush(page_file_ptr);

        //     // initialize new page
        //     page = new Page();
        //     init_fixed_len_page(page, page_size, ATTR_SIZE * ATTR_NUM);

        //     add_fixed_len_page(page, record);
        // }

        for (int i = 0; i < record->size(); i++) {
            // printf("%s", record->at(i));
            // printf("\n");
        }
    }
    fclose(csv_file_ptr);
    fclose(page_file_ptr);

    printf("%s\n", csv_file_name);
    printf("%s\n", page_file_name);
    printf("%li\n", page_size);
}

