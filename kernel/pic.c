#include "./include/pic.h"
#include "./include/ioutil.h"
void init_pic()
{
    // 初始化主片
    outb(PIC_MASTER_CMD, 0x11);  // ICW1
    outb(PIC_MASTER_DATA, 0x20); // ICW2
    outb(PIC_MASTER_DATA, 0x04); // ICW3
    outb(PIC_MASTER_DATA, 0x01); // ICW4
    // 初始化从片
    outb(PIC_SLAVE_CMD, 0x11);  // ICW1
    outb(PIC_SLAVE_DATA, 0x28); // ICW2
    outb(PIC_SLAVE_DATA, 0x02); // ICW3
    outb(PIC_SLAVE_DATA, 0x01); // ICW4
    // 打开主片上的IRQ0,也就是开启时钟中断
    outb(PIC_MASTER_DATA, 0xFE);
    // 关闭从片上的所有中断
    outb(PIC_SLAVE_DATA, 0xFF);
}