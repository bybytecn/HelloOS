#ifndef DEBUG_H
#define DEBUG_H
#include "../../common/include/types.h"
#include "./multiboot.h"
#include "./elf.h"
#define PANIC(...) panic_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)
/***********************************************************************/
#define ASSERT(CONDITION)                                                   \
    if (CONDITION)                                                          \
    {                                                                       \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        /* 符号#让编译器将宏的参数转化为字符串字面量 */ \
        PANIC(#CONDITION);                                                  \
    }
struct debug_elf_tab_t
{
    elf_symbol_t *symtab;
    uint32_t symtabsz;
    const char *strtab;
    uint32_t strtabsz;
};
void panic(char *str);
void print_stack_trace();
void panic_spin(char *filename,
                int line,
                const char *func,
                const char *condition);

void init_debug(struct multiboot_t *mtb);
const char *lookup_sym(uint32_t addr);
#endif