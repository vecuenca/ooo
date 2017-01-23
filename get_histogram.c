#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "library.h"

// todo handle error case, send -1 
int get_histogram(
	FILE *file_ptr,
	long hist[],
	int block_size,
	long *milliseconds,
	long *total_bytes_read)
{
	// Initialize our buffer.
	char buf[block_size];
	bzero(buf, block_size);

	// Setup variables to be used in our while loop
	long bytes_read = 0; 
	long i; // Used to iterate through for loop
	int character_read; // Used to store the character read

	// Begin timing
	long start_time = getTime();

	// Iterate until fread returns 0.
	while (bytes_read = fread(&buf, 1, block_size, file_ptr)) {
		*total_bytes_read = *total_bytes_read + bytes_read;

		// Go through every single character in the buffer.
		for (i = 0; i < bytes_read; i++) {
			character_read = (buf[i] - '0') - 17; // We first convert character to int, and then we translate A to 0
			
			if (0 <= character_read && character_read < 26) {
				hist[character_read]++;
			}
			// What to do if not a valid character?
		}
	}

	*milliseconds = (getTime() - start_time);

	fclose(file_ptr);

	return 0;
}

int main(int argc, const char* argv[])
{
	if (argc != 3)
	{
		printf("usage: ./get_histogram [filename] [blocksize]");
		exit(1);
	}

	// Setup our arguments
	const char *filename  = argv[1];
	long block_size       = strtol(argv[2], NULL, 10);

	// Setup args for get_histogram
	long hist[26];
	long milliseconds = 0;
	long filelen = 0;
	FILE *file_ptr = fopen(filename, "r");

	// Initialize our hist to 0
	int j;
	for (j = 0; j < 26; j++) {
		hist[j] = 0;
	}

	int ret = get_histogram(file_ptr, hist, block_size, &milliseconds, &filelen);

	if (ret < 0) {
		return 0;
	}

	printf("Computed the histogram in %d ms.\n", milliseconds);
	printf("File length: %ld\n", filelen);

	int i;
	for(i = 0; i < 26; i++) {
    	printf("%c : %d\n", 'A' + i, hist[i]);
	}
	printf("Data rate: %f Bps\n", (double)filelen/milliseconds * 1000);
}