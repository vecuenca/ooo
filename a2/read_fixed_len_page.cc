#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <iterator>

#include "library.h"

std::string tokenize(const std::string& s) {
   if (!s.size()) {
     return "";
   }
   std::stringstream ss;
   ss << s[0];
   for (int i = 1; i < s.size(); i++) {
     if (i % 10 == 0) ss << ',';
     ss << s[i];
   }
   return ss.str();
}

int main(int argc, const char *argv[]) {
	if (argc != 3) {
		printf("usage ./read_fixed_len_page [page_file] [page_size]");
		exit(1);
	}

    // Start timing
    long total_time_elapsed = 0;
    long write_start_time = getTime();

	const char *page_file_name = argv[1];
	long page_size = strtol(argv[2], NULL, 10);
	FILE *page_file_ptr = fopen(page_file_name, "r");

	Page* page = new Page();
	init_fixed_len_page(page, page_size, ATTR_NUM * ATTR_SIZE);

	Record* record = new Record();

	int amountRead;
	char* line_buffer = new char[ATTR_NUM * ATTR_SIZE + 99];
	
	while((amountRead = fread(page->data, sizeof(char), page_size, page_file_ptr)) > 0) {
		printf("%i, %i\n", amountRead, fixed_len_page_capacity(page));
		for (int i = 0; i < fixed_len_page_capacity(page); i++) {
			fixed_len_read(((char *) page->data) + i * fixed_len_sizeof(record), fixed_len_sizeof(record), record);

			memset(line_buffer, '\0', ATTR_NUM * ATTR_SIZE + 99);
			for (int j = 0; j < ATTR_NUM; j++) {
				// print out each attribute
				strncat(line_buffer, record->at(j), ATTR_SIZE);
				if (j != ATTR_NUM - 1) {
					strncat(line_buffer, ",", 1);
				}

				printf("%s", line_buffer);
			}
		}
	}

	total_time_elapsed = getTime();
	printf("Time: %ld milliseconds\n", total_time_elapsed - write_start_time);
}