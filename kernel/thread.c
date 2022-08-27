#include "./include/thread.h"
#include "./include/mm.h"
#include "./include/debug.h"
#include "../common/include/strings.h"
uint32_t g_tid_increase = 0;
uint32_t g_main_thread_stack_top;
// struct thread_queue_t ready_queue;   // 就绪队列
// struct thread_queue_t io_queue;      // IO等待队列
// struct thread_queue_t sleep_queue;   // 睡眠队列
struct thread_queue_t blocked_queue; //阻塞队列
struct thread_queue_t running_queue; // 运行队列
static void init_queue(struct thread_queue_t *queue);
static void push_queue(struct thread_queue_t *queue, struct thread_t *thread);
static struct thread_node_t *pop_queue(struct thread_queue_t *queue);
static struct thread_node_t *front_queue(struct thread_queue_t *queue);
static bool is_queue_empty(struct thread_queue_t *queue);
static void append_rear_queue(struct thread_queue_t *queue, struct thread_node_t *node);
static void append_front_queue(struct thread_queue_t *queue, struct thread_node_t *node);
// 初始化主线程
static void init_main_thread()
{
    // get esp into g_main_thread_stack_top
    asm volatile("mov %%esp, %0"
                 : "=r"(g_main_thread_stack_top));
    g_main_thread_stack_top = (g_main_thread_stack_top & 0xFFFFF000) | 0xfff;
    struct thread_t *main = alloc(sizeof(struct thread_t));
    main->tid = find_free_tid();
    main->esp = g_main_thread_stack_top;
    main->cs = SELECTOR_KERNEL_CODE;
    main->ds = SELECTOR_KERNEL_DATA;
    main->es = SELECTOR_KERNEL_DATA;
    main->fs = SELECTOR_KERNEL_DATA;
    main->gs = SELECTOR_KERNEL_DATA;
    main->ss = SELECTOR_KERNEL_DATA;

    strcpy(main->name, "main");
    main->status = THREAD_RUNNING;
    main->ticket = TICKET_KERNEL;
    push_queue(&running_queue, main);
}

void init_schduler()
{
    // init_queue(&ready_queue);
    // init_queue(&io_queue);
    // init_queue(&sleep_queue);
    init_queue(&running_queue);
    init_queue(&blocked_queue);

    init_main_thread();
}
static void init_queue(struct thread_queue_t *queue)
{
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}
static bool is_queue_empty(struct thread_queue_t *queue)
{
    return queue->size == 0;
}
// 新建一个结点添加到队尾
static void push_queue(struct thread_queue_t *queue, struct thread_t *thread)
{
    // new node
    struct thread_node_t *node = (struct thread_node_t *)alloc(sizeof(struct thread_node_t));
    ASSERT(node != 0);
    node->thread = thread;
    node->next = 0;
    node->prev = 0;

    // push
    if (queue->size == 0)
    {
        queue->head = node;
        queue->tail = node;
    }
    else
    {
        queue->tail->next = node;
        node->prev = queue->tail;
        queue->tail = node;
    }
    queue->size++;
}

// 追加结点到队尾
static void append_rear_queue(struct thread_queue_t *queue, struct thread_node_t *node)
{
    // push
    if (queue->size == 0)
    {
        queue->head = node;
        queue->tail = node;
    }
    else
    {
        queue->tail->next = node;
        node->prev = queue->tail;
        queue->tail = node;
    }
    queue->size++;
}

// 从队头取出一个结点
static struct thread_node_t *pop_queue(struct thread_queue_t *queue)
{
    // pop
    struct thread_node_t *node = queue->head;
    struct thread_t *thread = node->thread;
    if (queue->size == 1)
    {
        queue->head = 0;
        queue->tail = 0;
    }
    else
    {
        queue->head = node->next;
        queue->head->prev = 0;
    }
    queue->size--;
    // ASSERT(free(node))
    return node;
}

// 返回队头结点
static struct thread_node_t *front_queue(struct thread_queue_t *queue)
{
    return queue->head;
}

// 追加结点到队头
static void append_front_queue(struct thread_queue_t *queue, struct thread_node_t *node)
{
    // push
    if (queue->size == 0)
    {
        queue->head = node;
        queue->tail = node;
    }
    else
    {
        node->next = queue->head;
        queue->head->prev = node;
        queue->head = node;
    }
    queue->size++;
}

// 根据线程id查找线程
static struct thread_node_t *element_queue_tid(struct thread_queue_t *queue, uint32_t tid)
{
    struct thread_node_t *node = queue->head;
    while (node != 0)
    {
        if (node->thread->tid == tid)
        {
            return node;
        }
        node = node->next;
    }
#ifdef DEBUG
    printf("not found tid:%d\n", tid);
#endif
    return 0;
}

// 从队列中删除结点
static void remove_node_queue(struct thread_queue_t *queue, struct thread_node_t *t)
{
    if (t->prev)
    {
        t->prev->next = t->next;
        if (t->next)
        {
            t->next->prev = t->prev;
        }
        else
        {
            queue->tail = t->prev;
        }
        queue->size--;
    }
    else
    {
        ASSERT(t == pop_queue(queue));
    }
}

// 8253中断时执行此函数，调度线程
void schdule(uint32_t esp, uint32_t ebp, uint32_t edi, uint32_t esi, uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, uint32_t eip, uint32_t cs, uint32_t eflags)
{
    uint32_t ds, es, fs, gs, ss;
    asm volatile("movl %%ds, %0"
                 : "=r"(ds));
    asm volatile("movl %%es, %0"
                 : "=r"(es));
    asm volatile("movl %%fs, %0"
                 : "=r"(fs));
    asm volatile("movl %%gs, %0"
                 : "=r"(gs));
    asm volatile("movl %%ss, %0"
                 : "=r"(ss));
    bool level_change = FALSE;
    if (cs & 3 == DPL_USER)
    {
        // 特权级发生变化
        level_change = TRUE;
    }
    if (FALSE == level_change)
    {
        // 这个才是中断前的esp的值
        asm volatile("nop");
        esp += (48);
        uint32_t stack_top = esp & 0xFFFFF000;
        struct thread_node_t *p = running_queue.head;
        while (p)
        {
            if ((p->thread->esp & 0xFFFFF000) == stack_top)
            {
                // 更新上下文
                p->thread->esp = esp;
                p->thread->eax = eax;
                p->thread->ebx = ebx;
                p->thread->ecx = ecx;
                p->thread->edx = edx;
                p->thread->esi = esi;
                p->thread->edi = edi;
                p->thread->ebp = ebp;
                p->thread->eflags = eflags;
                p->thread->eip = eip;
                p->thread->cs = cs;
                p->thread->ds = ds;
                p->thread->es = es;
                p->thread->fs = fs;
                p->thread->gs = gs;
                p->thread->ss = ss;
                p->thread->ticket--;
                break;
            }
            p = p->next;
        }
#ifdef DEBUG
        kprintf("cur %s ", p->thread->name);
#endif
        ASSERT((is_queue_empty(&running_queue) == FALSE));
        p = running_queue.head;
        struct thread_t *t = 0;
        while (TRUE)
        {
            struct thread_node_t *tmp = front_queue(&running_queue);
            if (tmp->thread->status == THREAD_RUNNING)
            {
                if (0 == tmp->thread->ticket)
                {
                    //时间片用完
                    struct thread_node_t *t = pop_queue(&running_queue); //不释放内存，因为还需要用

                    t->thread->ticket = TICKET_KERNEL;

                    // 放到running_queue队尾
                    append_rear_queue(&running_queue, t);
                    continue;
                }
                t = tmp->thread;
                break;
            }
            else
            {
                PANIC("thread status error");
            }
        }
#ifdef DEBUG
        kprintf("next %s \n", t->name);
#endif
        ASSERT(t->ticket != 0)
        ASSERT(t != 0);
        *((uint32_t *)(t->esp - 4)) = t->eflags;
        *((uint32_t *)(t->esp - 8)) = t->cs;
        *((uint32_t *)(t->esp - 12)) = t->eip;
        switch_r0(t->ss, t->gs, t->es, t->fs, t->ds, t->ebp, t->edi, t->esi, t->edx, t->ecx, t->ebx, t->eax, t->esp);
        // never reach here
        PANIC("never reach here");
    }
    uint32_t esp3 = 0;
    uint32_t ss3 = 0;
    esp3 = *((uint32_t *)esp);
    ss3 = *((uint32_t *)(esp + 4));
    return;
}

// 当前的特权级
uint32_t get_running_cpl()
{
    uint32_t cpl;
    asm volatile("movl %%cs, %0"
                 : "=r"(cpl));
    return cpl & 0x3;
}
uint32_t find_free_tid()
{
    return ++g_tid_increase;
}

// 启动线程  需要关中断或者加锁使用，不能被打断
void create_thread(char *name, void *entry)
{
    struct thread_t *th = alloc(sizeof(struct thread_t));
    // TODO 需要支持4k页对齐的分配函数
    uint32_t esp = ((uint32_t)alloc(THREAD_STACK_SIZE)) & 0xfffff000;
    // ASSERT(0 == (esp & 0xfff));
    th->tid = find_free_tid();
    th->esp = (esp + THREAD_STACK_SIZE - 1);
    th->eip = (uint32_t)entry;
    th->cs = SELECTOR_KERNEL_CODE;
    th->ds = SELECTOR_KERNEL_DATA;
    th->es = SELECTOR_KERNEL_DATA;
    th->fs = SELECTOR_KERNEL_DATA;
    th->gs = SELECTOR_KERNEL_DATA;
    th->ss = SELECTOR_KERNEL_DATA;
    th->eflags = 0x293; //随便设置的，主要是IF位要打开

    strcpy(th->name, name);
    th->status = THREAD_RUNNING;
    if (get_running_cpl() == DPL_KERNEL)
    {
        th->ticket = TICKET_KERNEL;
    }
    else
    {
        th->ticket = TICKET_USER;
    }
    push_queue(&running_queue, th);
}

// 返回当前的线程，需要关中断或者加锁使用，不能被打断
struct thread_node_t *running_thread()
{
    uint32_t esp = 0;
    asm volatile("movl %%esp, %0"
                 : "=r"(esp));
    uint32_t stack_top = esp & 0xFFFFF000;
    struct thread_node_t *p = running_queue.head;
    ASSERT((p->thread->esp & 0xFFFFF000) == stack_top)
    //     while (p)
    //     {
    //         if ((p->thread->esp & 0xFFFFF000) == stack_top)
    //         {
    //             return p;
    //         }
    //         p = p->next;
    //     }
    // #ifdef DEBUG
    //     PANIC("running_thread: not found");
    // #endif
    return p;
}

// 线程退出时需要调用
void exit_thread()
{
    asm volatile("cli");
    //在运行队列中找到结点，修改状态
    struct thread_node_t *t = running_thread();
    ASSERT(0 != t);
    //删除该结点
    remove_node_queue(&running_queue, t);
    // 释放内存
    free(t->thread);
    free(t);
    asm volatile("sti");
}

void init_lock(struct lock_t *lock)
{
    lock->semaphore = 1;
    lock->size = 0;
    lock->front = 0;
    lock->rear = 0;
}

// 阻塞当前线程
void block()
{
    asm volatile("cli");
    struct thread_node_t *t = running_thread();
    //  从运行队列中移除
    remove_node_queue(&running_queue, t);
    // 加入到阻塞队列里
    append_rear_queue(&blocked_queue, t);
    to_schdule();
}

// 清空当前线程的时间片  需关中断
void clean_cur_cpu_ticket()
{
    struct thread_node_t *tt = running_thread();
    tt->thread->ticket = 1;
}

// 停止当前线程，运行指定线程 需关中断
void resume_thread_with_tid(uint32_t tid)
{
    clean_cur_cpu_ticket();
    struct thread_node_t *t = element_queue_tid(&blocked_queue, tid);
    if (t)
    {
        t->thread->ticket = 1;
        remove_node_queue(&blocked_queue, t);
        append_front_queue(&running_queue, t);
        to_schdule();
        return;
    }
    PANIC("resume_thread_with_tid: not found");
}

uint32_t pop_lock(struct lock_t *lock)
{
    if (lock->size == 0)
    {
        return 0;
    }
    lock->size--;
    uint32_t ret = lock->front;
    lock->front = (lock->front + 1) % LOCK_SIZE;
    return lock->waiter_tid[ret];
}
uint32_t append_lock(struct lock_t *lock, uint32_t tid)
{
    if (lock->size == LOCK_SIZE)
    {
        return 0;
    }
    lock->waiter_tid[lock->rear] = tid;
    lock->rear = (lock->rear + 1) % LOCK_SIZE;
    lock->size++;
    return 1;
}

// 加锁
void lock(struct lock_t *lock)
{
    asm volatile("cli");
    if (lock->semaphore > 0)
    {
        lock->semaphore--;
    }
    else
    {
        struct thread_node_t *t = running_thread();
        append_lock(lock, t->thread->tid);
        block();
        asm volatile("cli");
        ASSERT(lock->semaphore > 0);
        lock->semaphore--;
    }
    asm volatile("sti");
}

// 解锁
void unlock(struct lock_t *lock)
{
    asm volatile("cli");
    lock->semaphore++;
    uint32_t tmp = 0;
    if (lock->size > 0)
    {
        tmp = pop_lock(lock);
    }
    if (tmp != 0)
    {
        resume_thread_with_tid(tmp);
    }
}