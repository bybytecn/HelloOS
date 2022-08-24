#include "../kernel/include/console.h"
#include "../kernel/include/multiboot.h"
#include "../kernel/include/debug.h"
#include "../kernel/include/mm.h"
#include "../kernel/include/idt.h"
#include "../kernel/include/pic.h"
#include "../kernel/include/timer.h"
#include "../kernel/include/thread.h"
int entry(struct multiboot_t *mtb)
{
    init_debug(mtb);
    init_mm(mtb);       //初始化内存管理
    init_vm(get_cr3()); //初始化内核用的虚拟内存空间
    init_idt();
    init_pic();
    init_timer();
    init_schduler();
    kprintf("HelloOS");

    asm volatile("sti");
    while (1)
    {
        asm volatile("hlt");
    };
    return 0;
}