#include "../kernel/include/console.h"
#include "../kernel/include/multiboot.h"
#include "../kernel/include/debug.h"
#include "../kernel/include/mm.h"
#include "../kernel/include/idt.h"
#include "../kernel/include/pic.h"
#include "../kernel/include/timer.h"
#include "../kernel/include/thread.h"
void test()
{
    int sum = 0;
    for (int i = 0; i < 30000; i++)
    {
        sum += i;
        kprintf("2 %d\n", sum);
    }
    kprintf("1 %d\n", sum);
    exit_thread();
}
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
    create_thread("test", test);
    asm volatile("sti");
    while (1)
    {
        // kprintf("1\n");
        asm volatile("hlt");
    };
    return 0;
}