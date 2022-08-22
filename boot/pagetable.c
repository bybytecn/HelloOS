#include "./include/pagetable.h"
#include "../boot/include/gdt.h"

struct pde_t *g_kernel_pde __attribute__((section(".init_data")));
struct pte_t *g_kernel_pte __attribute__((section(".init_data")));
void __attribute__((section(".init_text"))) init_pagetable()
{
    g_kernel_pde = (struct pde_t *)0x8000;
    g_kernel_pte = (struct pte_t *)0x9000;
    uint32_t phys_page = 0;
    for (int i = 0; i < 1024; i++)
    {
        g_kernel_pde[i].present = 0;
    }
    //映射0-4MB的内存
    for (int i = 0; i < 1024; i++)
    {
        g_kernel_pte[i].present = 1;
        g_kernel_pte[i].base = phys_page >> 12;
        phys_page += 4096;
    }
    g_kernel_pde[0].present = 1;
    g_kernel_pde[0].base = ((uint32_t)&g_kernel_pte[0]) >> 12;

    //高1G的0-4MB内存映射到低1G0-4MB
    g_kernel_pde[0x300].present = 1;
    g_kernel_pde[0x300].base = g_kernel_pde[0].base;

    //加载页目录表
    asm volatile("mov %0, %%cr3" ::"r"(g_kernel_pde));
    //开启分页
    asm volatile("mov %%cr0, %%eax;"
                 "or $0x80000000, %%eax;"
                 "mov %%eax, %%cr0;"
                 :
                 :
                 : "eax");

    // 重新加载gdt
    struct gdt_ptr_t gdt_ptr;
    // 获取gdt
    asm volatile("sgdt %0"
                 : "=m"(gdt_ptr));
    gdt_ptr.base += 0xC0000000;

    asm volatile("lgdt %0"
                 :
                 : "m"(gdt_ptr));
}