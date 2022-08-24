#ifndef THREAD_H
#define THREAD_H
#define THREAD_STACK_SIZE 4096
#include "../../common/include/types.h"
#include "../../boot/include/gdt.h"
#define TICKET_USER 0x100 // R3线程默认的时间片
#define TICKET_KERNEL 0x200
#define DPL_KERNEL 0
#define DPL_USER 3
enum thread_status
{
    THREAD_RUNNING, // 线程正在运行
    THREAD_IO,      // 线程正在阻塞中，等待IO
    THREAD_SLEEP,   // 线程正在阻塞中，等待唤醒
    THREAD_DIED,    // 线程已经死亡
};
struct thread_t
{
    uint32_t tid;
    char name[10];
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp, esp;
    uint32_t eip, eflags;
    uint32_t cs, ds, es, fs, gs, ss;
    uint32_t ticket; // 线程的时间片
    enum thread_status status;

    uint32_t io_data; // 线程阻塞时，用于保存阻塞的数据地址
    uint32_t io_size; // 线程阻塞时，用于保存阻塞的数据大小
};
struct thread_node_t
{
    struct thread_t *thread;
    struct thread_node_t *next, *prev;
};

struct thread_queue_t
{
    struct thread_node_t *head, *tail;
    uint32_t size;
};

void init_schduler();

uint32_t get_running_cpl();

void switch_r0(uint32_t esp, uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi, uint32_t ebp, uint32_t ds, uint32_t fs, uint32_t es, uint32_t gs, uint32_t ss);
#endif