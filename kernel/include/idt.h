#ifndef IDT_H
#define IDT_H
#include "../../common/include/types.h"
#define IDT_NUM 81
typedef struct __attribute__((packed)) interupt_gate
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t always0;
    uint8_t flags;
    uint16_t offset_high;
} interupt_gate;
typedef struct __attribute__((packed)) interupt_descriptor
{
    uint16_t limit;
    uint32_t base;
    struct interupt_gate idt[IDT_NUM];
} interupt_descriptor;

void init_idt();
#endif