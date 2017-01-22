#include <stdio.h>
#include <stdlib.h>
#include "library.h"

int main(int argc, const char* argv[])
{
	if (argc != 4)
	{
		printf("usage: ./create_random_file [filename] [bytes] [blocksize]");
		exit(1);
	}

	// Setup our arguments
	const char *filename  = argv[1];
	long total_bytes      = strtol(argv[2], NULL, 10);
	long block_size       = strtol(argv[3], NULL, 10);

	// Allocate our buffer size
	char *buf;
	buf = (char *) malloc(block_size);

	FILE *fp = fopen(filename, "w");

	long bytes_written = 0;
	long total_time_elapsed = 0;
	long bytes_to_write, write_start_time;
	
	while (bytes_written < total_bytes) {
		
		if (total_bytes - bytes_written < block_size) {
			bytes_to_write = total_bytes - bytes_written;
		} else {
			bytes_to_write = block_size;
		}

		// Populate our buffer with new random content
		random_array(buf, bytes_to_write);
		
		// Start timing how long it takes to write to disk
		write_start_time = getTime();
		fwrite(buf, 1, bytes_to_write, fp);
		fflush(fp);
		total_time_elapsed += (getTime() - write_start_time);
 
		bytes_written += bytes_to_write;
	}

	fclose(fp);
	free(buf);
	
	printf("%ld", total_time_elapsed);
	return total_time_elapsed;
}
