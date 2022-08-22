#include "./include/debug.h"
#include "./include/console.h"
#include "../common/include/strings.h"
struct debug_elf_tab_t g_debug_elf_tab;
void panic(char *str)
{
    asm volatile("cli");
    console_write_color("\n\nPANIC: ", rc_black, rc_red);
    console_write_color(str, rc_black, rc_red);
    console_write_color("\n\n", rc_black, rc_red);
    print_stack_trace();
    while (1)
        ;
}
void panic_spin(char *filename,
                int line,
                const char *func,
                const char *condition)
{
    asm volatile("cli");
    kprintf("\n\n\n!!!!! error !!!!!\n");
    kprintf("filename:");
    kprintf(filename);
    kprintf("\n");
    kprintf("line:%d", line);
    kprintf("\n");
    kprintf("function:");
    kprintf((char *)func);
    kprintf("\n");
    kprintf("condition:");
    kprintf((char *)condition);
    kprintf("\n");
    while (1)
        ;
}
void print_stack_trace()
{
    uint32_t *ebp, *eip;

    asm volatile("mov %%ebp, %0"
                 : "=r"(ebp));
    while (ebp)
    {
        eip = ebp + 1;
        if (*eip == 0)
        {
            break;
        }
        kprintf("   [%x] %s\n", *eip, lookup_sym(*eip));
        ebp = (uint32_t *)*ebp;
    }
}
void init_debug(struct multiboot_t *mtb)
{
    elf_section_header_t *sec = (elf_section_header_t *)(mtb->addr + 0xc0000000);
    uint32_t nametable = sec[mtb->shndx].addr;
    for (int i = 0; i < mtb->num; i++)
    {
        char *name = (char *)(sec[i].name + ((uint32_t)nametable) + 0xc0000000);
        if (strcmp(name, ".symtab") == 0)
        {
            g_debug_elf_tab.symtab = (struct elf_symbol_t *)(sec[i].addr + 0xc0000000);
            g_debug_elf_tab.symtabsz = sec[i].size;
        }
        else if (strcmp(name, ".strtab") == 0)
        {
            g_debug_elf_tab.strtab = (const char *)(sec[i].addr + 0xc0000000);
            g_debug_elf_tab.strtabsz = sec[i].size;
        }
    }
}
const char *lookup_sym(uint32_t addr)
{
    if (0 == addr)
    {
        return "NULL";
    }
    for (int i = 0; i < g_debug_elf_tab.symtabsz / (sizeof(struct elf_symbol_t)); i++)
    {
        if (addr >= (g_debug_elf_tab.symtab[i].value) && addr < (g_debug_elf_tab.symtab[i].value + g_debug_elf_tab.symtab[i].size))
        {
            return (char *)(g_debug_elf_tab.strtab + g_debug_elf_tab.symtab[i].name);
        }
    }
    return 0;
}