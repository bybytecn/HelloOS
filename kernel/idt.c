#include "./include/idt.h"
#include "../boot/include/gdt.h"
#include "./include/console.h"
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
void idt_r0_handler(uint32_t esp, uint32_t ebp, uint32_t edi, uint32_t esi, uint32_t edx, uint32_t ecx, uint32_t ebx, uint32_t eax, uint32_t vecNum, uint32_t errCode, uint32_t eip, uint32_t cs, uint32_t eflags)
{
    // kprintf("vecNum:%d, errCode:%d, eip:%x, cs:%x, eflags:%x \n", vecNum, errCode, eip, cs, eflags);
}