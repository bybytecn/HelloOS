#include "./include/idt.h"
#include "../boot/include/gdt.h"
extern int g_idt_table;
struct interupt_descriptor g_idt_desc;
void init_idt()
{
    for (int i = 0; i < IDT_NUM; i++)
    {
        uint32_t base = *((&g_idt_table) + i);
        g_idt_desc.idt[i].offset_low = base & 0xFFFF;
        g_idt_desc.idt[i].offset_high = (base >> 16) & 0xFFFF;
        g_idt_desc.idt[i].selector = SELECTOR_KERNEL_CODE;
        g_idt_desc.idt[i].always0 = 0;
        g_idt_desc.idt[i].flags = 0x8E; //只允许DPL=0的特权级访问
    }
    g_idt_desc.base = (uint32_t) & (g_idt_desc.idt);
    g_idt_desc.limit = (sizeof(struct interupt_gate) * IDT_NUM) - 1;
    asm volatile("lidt %0"
                 :
                 : "m"(g_idt_desc));
}
// 特权级没发生变化时用的处理函数
void idt_r0_handler(int esp, int ebp, int edi, int esi, int edx, int ecx, int ebx, int eax, int vecNum, int errCode, int eip, int cs, int eflags)
{
}