#include "../kernel/include/console.h"
#include "../kernel/include/multiboot.h"
#include "../kernel/include/debug.h"
#include "../kernel/include/mm.h"
int entry(struct multiboot_t *mtb)
{
    init_debug(mtb);
    init_mm(mtb);
    uint32_t *ptr = (uint32_t *)alloc_phy(0, 4);
    ptr = (uint32_t *)(TOHM((uint32_t)(ptr)));
    *ptr = 1;
    set_vm_attr(((uint32_t)ptr), get_cr3(), PAGE_USER_ACCESS | PAGE_WRITE);

    bool vma = alloc_phy_for_vma(0x3f000000, get_cr3());
    uint32_t *x = (uint32_t *)0x3f000000;
    *x = 111;
    kprintf("HelloOS:%x %x\n!!!", ptr, *x);

    uint32_t cr3 = create_cr3();
    switch_cr3((cr3));
    while (1)
    {
        asm volatile("hlt");
    };
    return 0;
}