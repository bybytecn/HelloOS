#include "../kernel/include/console.h"
#include "../kernel/include/multiboot.h"
#include "../kernel/include/debug.h"
#include "../kernel/include/mm.h"
int entry(struct multiboot_t *mtb)
{
    init_debug(mtb);
    init_mm(mtb);
    uint32_t *ptr = (uint32_t *)alloc(0, 4);
    ptr = (uint32_t *)(TOHM((uint32_t)(ptr)));
    *ptr = 1;
    set_vm_attr(((uint32_t)ptr), get_cr3(), PAGE_USER_ACCESS | PAGE_WRITE);
    kprintf("HelloOS:%x \n!!!", ptr);
    while (1)
    {
        asm volatile("hlt");
    };
    return 0;
}