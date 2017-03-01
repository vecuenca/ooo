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

	const char *page_file_name = argv[1];
	long page_size = strtol(argv[2], NULL, 10);
	FILE *page_file_ptr = fopen(page_file_name, "r");
	char *record_str = new char[ATTR_NUM * ATTR_SIZE];

	// read one record at a time
	while(fgets(record_str, ATTR_SIZE * ATTR_NUM, page_file_ptr) != NULL) {
		// split into 10 byte attrs split by commas
		std::string comma_string = tokenize(record_str);
		printf("%s\n", comma_string);
	}
}