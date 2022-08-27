#include "../kernel/include/console.h"
#include "../kernel/include/multiboot.h"
#include "../kernel/include/debug.h"
#include "../kernel/include/mm.h"
#include "../kernel/include/idt.h"
#include "../kernel/include/pic.h"
#include "../kernel/include/timer.h"
#include "../kernel/include/thread.h"
struct lock_t lk;
uint32_t testg = 3;
void test()
{
    while (1)
    {
        lock(&lk);
        if (testg % 2 == 0)
        {
            kprintf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
        }
        testg++;
        for (int z = 0; z < 100000; z++)
            ;
        testg--;
        kprintf("11\n");
        unlock(&lk);
    };
    exit_thread();
}
void test2()
{
    while (1)
    {
        lock(&lk);
        if (testg % 2 == 0)
        {
            kprintf("111111111111111111111111111111111111111111111\n");
        }
        kprintf("33333333333333333333333333333\n");
        unlock(&lk);
    };
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
    create_thread("test2", test2);
    init_lock(&lk);
    asm volatile("sti");
    while (1)
    {
        lock(&lk);
        testg++;
        for (int z = 0; z < 100; z++)
            ;
        testg--;
        kprintf("222222222\n");
        unlock(&lk);
    };
    return 0;
}