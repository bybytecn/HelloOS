#ifndef MM_H
#define MM_H
#include "./multiboot.h"
#define MAX_FREE_BLOCK_NUM 528
#define MAX_ALLOC_BLOCK_NUM 528 * 2
#define TOHM(a) ((uint32_t)a + 0xc0000000) // 0-4M的内存转换到高1G上，否则会读取错误
#define KERNEL_LIMIT 0x400000              // 4M以上的内存没在页表内

#define PAGE_PRESENT 0x1
#define PAGE_WRITE 0x2
#define PAGE_USER_ACCESS 0x4

#define VM_BITMAP_PAGE_SIZE 4096                                       // 1 bit代表一页，共8K字节
#define VM_START_ADDR ((((0x100000000L) / (VM_BITMAP_PAGE_SIZE) / 8))) // 分配虚拟内存的起始地址, 也代表bitmap结构占用的大小

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
struct pde_t
{
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t pwt : 1;
    uint32_t pcd : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t pat : 1;
    uint32_t global : 1;
    uint32_t avail : 3;
    uint32_t base : 20;
};
struct pte_t
{
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t pwt : 1;
    uint32_t pcd : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t pat : 1;
    uint32_t global : 1;
    uint32_t avail : 3;
    uint32_t base : 20;
};
struct vm_controll_t
{
    uint32_t size;       //包括申请的大小和本控制头占用的大小 即实际占用的大小
    uint32_t start_addr; //给调用者使用的地址

    uint32_t start_bit_offset;
    uint32_t end_bit_offset;
};

// 物理内存管理
void init_mm(struct multiboot_t *m);
void add_memseg(uint32_t addr, uint32_t size);
void *alloc_phy(uint32_t start_addr, uint32_t size);
void *alloc_phy_4k(uint32_t start_addr, uint32_t size);
bool free_phy(uint32_t ptr);
uint32_t get_phy_total_free();
uint32_t get_phy_total_alloc();

// 虚拟内存管理
bool alloc_phy_for_vma(uint32_t vm_addr, uint32_t cr3);
bool free_phy_for_vma(uint32_t vm_addr, uint32_t cr3);
void alloc_phy_for_vma_range(uint32_t vm_addr, uint32_t size, uint32_t cr3);
void free_phy_for_vma_range(uint32_t vm_addr, uint32_t size, uint32_t cr3);
void set_vm_attr(uint32_t vm_addr, uint32_t cr3, uint32_t attr);
void init_vm(uint32_t cr3);
void *alloc_vm(uint32_t cr3, uint32_t size);
bool free_vm(uint32_t vm_addr, uint32_t cr3);

uint32_t get_cr3();
uint32_t create_cr3();
uint32_t clean_cr3(uint32_t cr3);

void switch_cr3(uint32_t cr3);
#endif