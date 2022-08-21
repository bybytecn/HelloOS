#include "./include/mm.h"
#include "./include/console.h"
extern uint8_t g_kern_start[];
extern uint8_t g_kern_end[];
void init_mm(struct multiboot_t *m)
{
    uint32_t mmap_addr = m->mmap_addr;
    uint32_t mmap_length = m->mmap_length;
    mmap_entry_t *mmap = (mmap_entry_t *)mmap_addr;
    for (mmap = (mmap_entry_t *)mmap_addr; (uint32_t)mmap < mmap_addr + mmap_length; mmap++)
    {
        uint64_t start = ((uint64_t)mmap->base_addr_high << 32) | mmap->base_addr_low;
        uint64_t size = ((uint64_t)mmap->length_high << 32) | mmap->length_low;
        if (start >= 0x100000L && mmap->type == 1)
        {
            if (start == 0x100000L)
            {
                uint64_t kenerl_size = ((int)g_kern_end - (int)g_kern_start);
                // add_memmg(start + kenerl_size, (uint64_t)(size - kenerl_size));
            }
            else
            {
                // add_memmg(start, size);
            }
        }
#ifdef DEBUG
        kprintf("base_addr = %X, length = %X type:%d \n", start, size, mmap->type);
#endif
    }
}