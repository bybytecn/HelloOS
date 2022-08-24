#ifndef GDT_H
#define GDT_H
#include "../../common/include/types.h"
#define SELECTOR_KERNEL_CODE 0x10
#define SELECTOR_KERNEL_DATA 0x18
#define SELECTOR_USER_CODE 0x20
#define SELECTOR_USER_DATA 0x28
#define SELECTOR_TSS 0x30
struct gdt_t
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));
struct gdt_ptr_t
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));
void init_gdt();
#endif