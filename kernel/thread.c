#include "./include/thread.h"
#include "./include/mm.h"
#include "./include/debug.h"
#include "../common/include/strings.h"
uint32_t g_main_thread_stack_top;
struct thread_queue_t ready_queue;   // 就绪队列
struct thread_queue_t io_queue;      // IO等待队列
struct thread_queue_t sleep_queue;   // 睡眠队列
struct thread_queue_t running_queue; // 运行队列
static void init_queue(struct thread_queue_t *queue);
static void push_queue(struct thread_queue_t *queue, struct thread_t *thread);
static struct thread_t *pop_queue(struct thread_queue_t *queue);
static struct thread_t *front_queue(struct thread_queue_t *queue);
static bool is_queue_empty(struct thread_queue_t *queue);

// 初始化主线程
static void init_main_thread()
{
    // get esp into g_main_thread_stack_top
    asm volatile("mov %%esp, %0"
                 : "=r"(g_main_thread_stack_top));
    g_main_thread_stack_top = g_main_thread_stack_top & 0xFFFFF000;
    struct thread_t *main = alloc(sizeof(struct thread_t));
    main->tid = 1;
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
static struct thread_t *pop_queue(struct thread_queue_t *queue)
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
    return thread;
}
static struct thread_t *front_queue(struct thread_queue_t *queue)
{
    return queue->head->thread;
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
        p = running_queue.head;
        if (0 == p->thread->ticket)
        {
            //时间片用完
            struct thread_t *t = pop_queue(&running_queue); //不释放内存，因为还需要用

            t->ticket = TICKET_KERNEL;
            push_queue(&running_queue, t);
        }
        struct thread_t *t = front_queue(&running_queue);
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