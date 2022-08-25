#include "./include/thread.h"
#include "./include/mm.h"
#include "./include/debug.h"
#include "../common/include/strings.h"
uint32_t g_tid_increase = 0;
uint32_t g_main_thread_stack_top;
struct thread_queue_t ready_queue;   // 就绪队列
struct thread_queue_t io_queue;      // IO等待队列
struct thread_queue_t sleep_queue;   // 睡眠队列
struct thread_queue_t running_queue; // 运行队列
static void init_queue(struct thread_queue_t *queue);
static void push_queue(struct thread_queue_t *queue, struct thread_t *thread);
static struct thread_node_t *pop_queue(struct thread_queue_t *queue);
static struct thread_node_t *front_queue(struct thread_queue_t *queue);
static bool is_queue_empty(struct thread_queue_t *queue);
static void append_node_queue(struct thread_queue_t *queue, struct thread_node_t *node);
// 初始化主线程
static void init_main_thread()
{
    // get esp into g_main_thread_stack_top
    asm volatile("mov %%esp, %0"
                 : "=r"(g_main_thread_stack_top));
    g_main_thread_stack_top = g_main_thread_stack_top & 0xFFFFF000;
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
    init_queue(&ready_queue);
    init_queue(&io_queue);
    init_queue(&sleep_queue);
    init_queue(&running_queue);

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
static void append_node_queue(struct thread_queue_t *queue, struct thread_node_t *node)
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
static struct thread_node_t *front_queue(struct thread_queue_t *queue)
{
    return queue->head;
}

// 8253中断时执行此函数，调度线程
void timer_handler(uint32_t esp, uint32_t ebp, uint32_t edi, uint32_t esi, uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, uint32_t eip, uint32_t cs, uint32_t eflags)
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
        esp += (12 * 4);
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
        ASSERT((is_queue_empty(&running_queue) == FALSE));
        p = running_queue.head;
        if (0 == p->thread->ticket)
        {
            //时间片用完
            struct thread_node_t *t = pop_queue(&running_queue); //不释放内存，因为还需要用

            t->thread->ticket = TICKET_KERNEL;

            // 放到running_queue队尾
            append_node_queue(&running_queue, t);
        }
        struct thread_node_t *tn = running_queue.head;
        struct thread_t *t = 0;
        // 选出下一个要运行的线程
        while (TRUE)
        {
            struct thread_node_t *tmp = front_queue(&running_queue);
            if (tmp->thread->status == THREAD_EXIT)
            {
                // 释放内存
                pop_queue(&running_queue);
                ASSERT(free(tmp));
                continue;
            }
            else if (tmp->thread->status == THREAD_RUNNING)
            {
                t = tmp->thread;
                break;
            }
            else
            {
                PANIC("thread status error");
            }
        }
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

// 需要关中断或者加锁使用，不能被打断
struct thread_t *running_thread()
{
    struct thread_t *r = 0;
    uint32_t esp = 0;
    asm volatile("movl %%esp, %0"
                 : "=r"(esp));
    uint32_t stack_top = esp & 0xFFFFF000;
    struct thread_node_t *p = running_queue.head;
    while (p)
    {
        if ((p->thread->esp & 0xFFFFF000) == stack_top)
        {
            r = p->thread;
            return r;
        }
        p = p->next;
    }
#ifdef DEBUG
    PANIC("running_thread: not found");
#endif
    return 0;
}

// 线程退出时需要调用
void exit_thread()
{
    asm volatile("cli");
    //在运行队列中找到结点，修改状态
    struct thread_t *t = running_thread();
    ASSERT(0 != t);
    //关中断
    t->status = THREAD_EXIT;
    //开中断
    asm volatile("sti");
    schdule();
}

// 放弃时间片，先让cpu处理其他任务
void schdule()
{
    asm volatile("cli");
    // 在运行队列中找到结点，修改状态
    struct thread_t *t = running_thread();
    ASSERT(0 != t);
    t->ticket = 0;
    asm volatile("sti");
    asm volatile("int $32");
}