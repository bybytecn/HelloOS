#ifndef MM_H
#define MM_H
#include "./multiboot.h"
#define MAX_FREE_BLOCK_NUM 128
#define MAX_ALLOC_BLOCK_NUM 128 * 3
#define TOHM(a) a + 0xc0000000 // 0-4M的内存转换到高1G上，否则会读取错误
/**
 * size是相关结构的大小，单位是字节，它可能大于最小值20
 * base_addr_low是启动地址的低32位，base_addr_high是高32位，启动地址总共有64位
 * length_low是内存区域大小的低32位，length_high是内存区域大小的高32位，总共是64位
 * type是相应地址区间的类型，1代表可用RAM，所有其它的值代表保留区域
 */
typedef struct mmap_entry_t
{
    uint32_t size; // 留意 size 是不含 size 自身变量的大小
    uint32_t base_addr_low;
    uint32_t base_addr_high;
    uint32_t length_low;
    uint32_t length_high;
    uint32_t type;
} __attribute__((packed)) mmap_entry_t;

struct mm_block_t
{
    uint32_t start; //所管理内存块的起始地址
    uint32_t size;
};
struct mm_manager_t
{
    struct mm_block_t free_area[MAX_FREE_BLOCK_NUM];
    struct mm_block_t alloc_area[MAX_ALLOC_BLOCK_NUM];
};
void init_mm(struct multiboot_t *m);
void add_memseg(uint32_t addr, uint32_t size);
void *alloc(uint32_t start_addr, uint32_t size);
void *alloc_4k(uint32_t start_addr, uint32_t size);
bool free(uint32_t ptr);
uint32_t get_total_free();
uint32_t get_total_alloc();
#endif