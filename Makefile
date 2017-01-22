CC = gcc
library.o: library.c library.h
	$(CC) -o library.o -c library.c
 
create_random_file: create_random_file.c library.o
	$(CC) -o $@ $< library.o
 
get_histogram: get_histogram.c library.o
	$(CC) -o $@ $< library.o
