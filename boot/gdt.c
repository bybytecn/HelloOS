#include "./include/gdt.h"
#include "../common/include/strings.h"
struct gdt_t g_gdt_table[6] __attribute__((section(".init_data")));
struct gdt_ptr_t g_gdt_entry __attribute__((section(".init_data")));
void __attribute__((section(".init_text"))) init_gdt()
{
    for (int i = 0; i < sizeof(g_gdt_table) / sizeof(struct gdt_t); i++)
    {
        g_gdt_table[i].limit_low = 0;
        g_gdt_table[i].base_low = 0;
        g_gdt_table[i].base_middle = 0;
        g_gdt_table[i].access = 0;
        g_gdt_table[i].limit_low = 0;
        g_gdt_table[i].base_high = 0;
    }

    g_gdt_table[SELECTOR_KERNEL_CODE >> 3].limit_low = 0xffff;
    g_gdt_table[SELECTOR_KERNEL_CODE >> 3].base_low = 0x0000;
    g_gdt_table[SELECTOR_KERNEL_CODE >> 3].base_middle = 0x00;
    g_gdt_table[SELECTOR_KERNEL_CODE >> 3].base_high = 0x00;
    g_gdt_table[SELECTOR_KERNEL_CODE >> 3].access = 0x9a; // DPL=0 Code, Readable, Accessed
    g_gdt_table[SELECTOR_KERNEL_CODE >> 3].granularity = 0xcf;

    g_gdt_table[SELECTOR_KERNEL_DATA >> 3].limit_low = 0xffff;
    g_gdt_table[SELECTOR_KERNEL_DATA >> 3].base_low = 0x0000;
    g_gdt_table[SELECTOR_KERNEL_DATA >> 3].base_middle = 0x00;
    g_gdt_table[SELECTOR_KERNEL_DATA >> 3].base_high = 0x00;
    g_gdt_table[SELECTOR_KERNEL_DATA >> 3].access = 0x92; // DPL=0 Data, Writable, Accessed
    g_gdt_table[SELECTOR_KERNEL_DATA >> 3].granularity = 0xcf;

    g_gdt_table[SELECTOR_USER_CODE >> 3].limit_low = 0xffff;
    g_gdt_table[SELECTOR_USER_CODE >> 3].base_low = 0x0000;
    g_gdt_table[SELECTOR_USER_CODE >> 3].base_middle = 0x00;
    g_gdt_table[SELECTOR_USER_CODE >> 3].base_high = 0x00;
    g_gdt_table[SELECTOR_USER_CODE >> 3].access = 0xfa; // DPL=3 Code, Readable, Accessed
    g_gdt_table[SELECTOR_USER_CODE >> 3].granularity = 0xcf;

    g_gdt_table[SELECTOR_USER_DATA >> 3].limit_low = 0xffff;
    g_gdt_table[SELECTOR_USER_DATA >> 3].base_low = 0x0000;
    g_gdt_table[SELECTOR_USER_DATA >> 3].base_middle = 0x00;
    g_gdt_table[SELECTOR_USER_DATA >> 3].base_high = 0x00;
    g_gdt_table[SELECTOR_USER_DATA >> 3].access = 0xf2; // DPL=3 Data, Writable, Accessed
    g_gdt_table[SELECTOR_USER_DATA >> 3].granularity = 0xcf;

    g_gdt_entry.limit = sizeof(g_gdt_table) - 1;
    g_gdt_entry.base = (uint32_t)g_gdt_table;

    // load gdt
    asm volatile("lgdtl (%0)" ::"r"(&g_gdt_entry));
}