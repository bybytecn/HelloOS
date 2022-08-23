#include "../kernel/include/console.h"
#include "../kernel/include/multiboot.h"
#include "../kernel/include/debug.h"
#include "../kernel/include/mm.h"
#include "../kernel/include/idt.h"
#include "../kernel/include/pic.h"
int entry(struct multiboot_t *mtb)
{
    init_debug(mtb);
    init_mm(mtb);
    init_idt();
    init_pic();
    kprintf("HelloOS");
    while (1)
    {
        asm volatile("hlt");
    };
    return 0;
}