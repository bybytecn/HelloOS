MBOOT_HEADER_MAGIC 	equ 	0x1BADB002 	; Multiboot 魔数，由规范决定的

MBOOT_PAGE_ALIGN 	equ 	1 << 0    	; 0 号位表示所有的引导模块将按页(4KB)边界对齐
MBOOT_MEM_INFO 		equ 	1 << 1    	; 1 号位通过 Multiboot 信息结构的 mem_* 域包括可用内存的信息
						; (告诉GRUB把内存空间的信息包含在Multiboot信息结构中)

; 定义我们使用的 Multiboot 的标记
MBOOT_HEADER_FLAGS 	equ 	MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO

; 域checksum是一个32位的无符号值，当与其他的magic域(也就是magic和flags)相加时，
; 要求其结果必须是32位的无符号值 0 (即magic + flags + checksum = 0)
MBOOT_CHECKSUM 		equ 	- (MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)
[GLOBAL start]
[EXTERN entry]
[EXTERN init_gdt]
[EXTERN init_pagetable]
[bits 32]
section .init_text

dd MBOOT_HEADER_MAGIC 	; GRUB 会通过这个魔数判断该映像是否支持
dd MBOOT_HEADER_FLAGS   ; GRUB 的一些加载时选项，其详细注释在定义处
dd MBOOT_CHECKSUM       ; 检测数值，其含义在定义处s

start:
    mov [ptr_mtb],ebx
    call init_gdt
    call init_pagetable

    mov eax,next
    add eax,0xc0000000
    jmp eax

next:
    ; 关掉0-4M的映射
    mov eax,cr3
    add eax,0xc0000000
    mov ebx,[eax]
    and ebx,0xfffffffe
    ; c0000000-0xc03fffff -> 0x00000000-0x003fffff
    mov [eax],ebx
    mov esp,stack-1
    mov ebp,0
    add esp,0xc0000000
    add ebp,0xc0000000
    mov ebx,[ptr_mtb+0xc0000000]
    add ebx,0xc0000000
    push ebx

    ; 从此都使用0xc0000000以上的地址代码、数据

    ; 直接call entry 好像有问题，，，
    mov eax,entry
    push 0
    jmp eax
section .init_text
    times 1024 db 0      ; 进入entry后使用的栈空间
    stack:
section .init_data
    ptr_mtb dd 0