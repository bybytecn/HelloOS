#include "../kernel/include/console.h"
#include "../kernel/include/multiboot.h"
#include "../kernel/include/debug.h"
int entry(struct multiboot_t *mtb)
{
    init_debug(mtb);
    kprintf("HelloOS!");
    while (1)
    {
        asm volatile("hlt");
    };
    return 0;
}