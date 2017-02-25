#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/timeb.h>

long getTime()
{
        struct timeb t;
        ftime(&t);
        
	return t.time * 1000 + t.millitm;
}

void random_array(char *array, long bytes)
{
	char c;
	long i;
	
	for (i = 0; i < bytes; i++) {
		c = 'A' + (rand() % 26);	
		array[i] = c;
	}
}

