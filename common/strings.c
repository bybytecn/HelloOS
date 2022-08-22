#include "./include/strings.h"
void memset(void *dst, uint8_t val, uint32_t size)
{
    uint8_t *p = (uint8_t *)dst;
    for (uint32_t i = 0; i < size; i++)
    {
        *p++ = val;
    }
}
int32_t strcmp(char *str1, char *str2)
{
    while (*str1 && *str2 && *str1 == *str2)
    {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}