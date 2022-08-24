#include "./include/thread.h"
#include "./include/mm.h"
#include "./include/debug.h"
enum task_status
{
    TASK_RUNNING, // 线程正在运行
    TASK_IO,      // 线程正在阻塞中，等待IO
    TASK_SLEEP,   // 线程正在阻塞中，等待唤醒
    TASK_DIED,    // 线程已经死亡
};

// 初始化主线程
void init_main_thread()
{
    uint32_t stack = (uint32_t)(alloc_vm(get_cr3(), KTHREAD_STACK_SIZE));
    uint32_t stack_top = (stack + KTHREAD_STACK_SIZE - 1) & 0xFFFFF000;
    uint32_t stack_base = stack;
    ASSERT(0 == (stack_top & 0xfff));
    // set esp
    asm volatile("movl %0, %%esp" ::"r"(stack_top));
}