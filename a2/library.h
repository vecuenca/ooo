#ifndef LIBRARY_H
#define LIBRARY_H  

int fixed_len_sizeof(Record *record);

void fixed_len_write(Record *record, void *buf);