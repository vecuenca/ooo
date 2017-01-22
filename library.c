#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

long getTime()
{
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;	
}

void random_array(char *array, long bytes)
{
	char c;	
	for (int i; i < bytes; i++) {
		c = 'A' + (rand() % 26);	
		array[i] = c;
	}
}

