#ifndef THREAD_H
#define THREAD_H
#define THREAD_STACK_SIZE 4096
#include "../../common/include/types.h"
#include "../../boot/include/gdt.h"
#define TICKET_USER 0x100 // R3线程默认的时间片
#define TICKET_KERNEL 0x1
#define DPL_KERNEL 0
#define DPL_USER 3
#define LOCK_SIZE 3
enum thread_status
{
    THREAD_RUNNING, // 线程正在运行
    THREAD_IO,      // 线程正在阻塞中，等待IO
    THREAD_SLEEP,   // 线程正在阻塞中，等待唤醒
    THREAD_EXIT,    // 线程已经死亡
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
struct lock_t
{
    uint32_t semaphore;
    uint32_t waiter_tid[LOCK_SIZE];
    uint32_t size;
    uint32_t front;
    uint32_t rear;
};
void create_thread(char *name, void *entry);

void init_schduler();

void exit_thread();

struct thread_node_t *running_thread();

uint32_t get_running_cpl();

uint32_t find_free_tid();

void to_schdule();

void block();

void lock(struct lock_t *lock);

void unlock(struct lock_t *lock);

void init_lock(struct lock_t *lock);

void switch_r0(uint32_t esp, uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi, uint32_t ebp, uint32_t ds, uint32_t fs, uint32_t es, uint32_t gs, uint32_t ss);
#endif