#include "./include/mm.h"
#include "./include/console.h"
#include "./include/debug.h"
extern uint8_t g_kern_start[];
extern uint8_t g_kern_end[];
uint32_t g_seg_cnt = 0;
struct mm_manager_t g_mm_manager;
void merge();
void init_mm(struct multiboot_t *m)
{
    uint32_t mmap_addr = TOHM(m->mmap_addr);
    uint32_t mmap_length = m->mmap_length;
    mmap_entry_t *mmap = (mmap_entry_t *)(mmap_addr);
    for (mmap = (mmap_entry_t *)mmap_addr; (uint32_t)mmap < mmap_addr + mmap_length; mmap++)
    {
        uint64_t start = ((uint64_t)mmap->base_addr_high << 32) | mmap->base_addr_low;
        uint64_t size = ((uint64_t)mmap->length_high << 32) | mmap->length_low;
        //只管理1M以上的内存
        if (start >= 0x100000L && mmap->type == 1)
        {
            //超过4G的内存不处理，因为寻址只有4GB
            if (start + size > 0x100000000L)
            {
                continue;
            }
            if (start == 0x100000L)
            {
                // 管理的内存不包括内核占用的内存
                uint32_t kenerl_size = ((int)g_kern_end - (int)g_kern_start);
                add_memseg((uint32_t)(start + kenerl_size), (uint32_t)(size - kenerl_size));
            }
            else
            {
                add_memseg((uint32_t)start, (uint32_t)size);
            }
        }
#ifdef DEBUG
        kprintf("base_addr = %X, length = %X type:%d \n", start, size, mmap->type);
#endif
    }
}

//  添加用于内存管理的段
void add_memseg(uint32_t addr, uint32_t size)
{
    g_mm_manager.free_area[g_seg_cnt].start = addr;
    g_mm_manager.free_area[g_seg_cnt].size = size;
    g_seg_cnt++;
}
// 分配4k对齐的内存，成功返回内存地址，失败返回0
void *alloc_4k(uint32_t size)
{
    merge();
    if (0 == size)
    {
        return 0;
    }
    int32_t idx = -1;
    uint32_t alloc_addr = 0;
    for (int i = 0; i < MAX_FREE_BLOCK_NUM; i++)
    {
        if (g_mm_manager.free_area[i].size >= size)
        {
            for (uint64_t k = g_mm_manager.free_area[i].start & 0xfffff000;
                 (k) < (g_mm_manager.free_area[i].start + g_mm_manager.free_area[i].size); k += 4096)
            {
                if ((k >= g_mm_manager.free_area[i].start) &&
                    (k + size < g_mm_manager.free_area[i].start + g_mm_manager.free_area[i].size))
                {
                    alloc_addr = k;
                    break;
                }
            }
            if (alloc_addr)
            {
                idx = i;
                break;
            }
        }
    }
    if (-1 == idx)
    {
//找不到合适的空间分配4k对齐的地址
#ifdef DEBUG
        kprintf("alloc_4k failed\n");
#endif
        return 0;
    }
    uint32_t free_size = alloc_addr - g_mm_manager.free_area[idx].start;
    uint32_t old_start_addr = g_mm_manager.free_area[idx].start;
    g_mm_manager.free_area[idx].start += (size + free_size);
    g_mm_manager.free_area[idx].size -= (size + free_size);

    if (free_size)
    {
        bool no_space = TRUE;
        //多余的内存块放到空闲块中
        for (int i = 0; i < MAX_FREE_BLOCK_NUM; i++)
        {
            if (g_mm_manager.free_area[i].size == 0)
            {
                g_mm_manager.free_area[i].start = old_start_addr;
                g_mm_manager.free_area[i].size = free_size;
                no_space = FALSE;
                break;
            }
        }
        if (no_space)
        {
#ifdef DEBUG
            panic("alloc_4k failed\n");
#endif
        }
    }
    //找到一个alloc块存放分配的内存块
    for (int i = 0; i < MAX_ALLOC_BLOCK_NUM; i++)
    {
        if (g_mm_manager.alloc_area[i].size == 0)
        {
            g_mm_manager.alloc_area[i].start = alloc_addr;
            g_mm_manager.alloc_area[i].size = size;
            return (void *)alloc_addr;
        }
    }
#ifdef DEBUG
    panic("alloc_4k failed\n");
#endif
}

// 合并分散的碎片块
void merge()
{
    // 按照起始地址排序
    for (int i = 0; i < MAX_FREE_BLOCK_NUM; i++)
    {
        for (int j = 0; j < MAX_FREE_BLOCK_NUM - 1; j++)
        {
            if (g_mm_manager.free_area[j].start > g_mm_manager.free_area[j + 1].start)
            {
                struct mm_block_t temp = g_mm_manager.free_area[j];
                g_mm_manager.free_area[j] = g_mm_manager.free_area[j + 1];
                g_mm_manager.free_area[j + 1] = temp;
            }
        }
    }
    // 合并成一大块
    for (int i = 0; i < MAX_FREE_BLOCK_NUM; i++)
    {
        if (g_mm_manager.free_area[i].size == 0)
        {
            continue;
        }
        for (int j = i + 1; j < MAX_FREE_BLOCK_NUM; j++)
        {
            if (g_mm_manager.free_area[j].size == 0)
            {
                continue;
            }
            if (g_mm_manager.free_area[i].start + g_mm_manager.free_area[i].size == g_mm_manager.free_area[j].start)
            {
                g_mm_manager.free_area[i].size += g_mm_manager.free_area[j].size;
                g_mm_manager.free_area[j].size = 0;
            }
        }
    }
}