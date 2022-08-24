#ifndef STRING_H
#define STRING_H
#include "./types.h"
void memset(void *dst, uint8_t val, uint32_t size);
int32_t strcmp(char *str1, char *str2);
int32_t strcpy(char *dst, char *src);
#endif