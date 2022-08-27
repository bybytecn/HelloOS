[bits 32]
extern idt_r0_handler;
global g_idt_table

section .data
g_idt_table:


%macro idt_r0 2
section .text
global idt_%1_entry
idt_%1_entry:

%if %2==0
push 0
%endif
;eflags, cs, eip, esp, eax, ebx, ecx, edx, esi, edi, ebp, ds, fs, es, gs, ss

push %1 ;Vector
push eax
push ebx
push ecx
push edx
push esi
push edi
push ebp
push esp

; ; 通知PIC继续处理中断
mov al,0x20
out 0x20,al
out 0xa0,al
call idt_r0_handler
pop esp
pop ebp
pop edi
pop esi
pop edx
pop ecx
pop ebx
pop eax
add esp,4*2
iret
section .data
dd idt_%1_entry
%endmacro

; 第一个是中断号、第二个表示是否有错误码
idt_r0 0,0
idt_r0 1,0
idt_r0 2,0
idt_r0 3,0
idt_r0 4,0
idt_r0 5,0
idt_r0 6,0
idt_r0 7,0
idt_r0 8,1
idt_r0 9,0
idt_r0 10,1
idt_r0 11,1
idt_r0 12,1
idt_r0 13,1
idt_r0 14,1
idt_r0 15,0
idt_r0 16,0
idt_r0 17,1
idt_r0 18,0
idt_r0 19,0
idt_r0 20,0
idt_r0 21,0
idt_r0 22,0
idt_r0 23,0
idt_r0 24,0
idt_r0 25,0
idt_r0 26,0
idt_r0 27,0
idt_r0 28,0
idt_r0 29,0
idt_r0 30,0
idt_r0 31,0
idt_r0 32,0
idt_r0 33,0
idt_r0 34,0
idt_r0 35,0
idt_r0 36,0
idt_r0 37,0
idt_r0 38,0
idt_r0 39,0
idt_r0 40,0
idt_r0 41,0
idt_r0 42,0
idt_r0 43,0
idt_r0 44,0
idt_r0 45,0
idt_r0 46,0
idt_r0 47,0
idt_r0 48,0
idt_r0 49,0
idt_r0 50,0
idt_r0 51,0
idt_r0 52,0
idt_r0 53,0
idt_r0 54,0
idt_r0 55,0
idt_r0 56,0
idt_r0 57,0
idt_r0 58,0
idt_r0 59,0
idt_r0 60,0
idt_r0 61,0
idt_r0 62,0
idt_r0 63,0
idt_r0 64,0
idt_r0 65,0
idt_r0 66,0
idt_r0 67,0
idt_r0 68,0
idt_r0 69,0
idt_r0 70,0
idt_r0 71,0
idt_r0 72,0
idt_r0 73,0
idt_r0 74,0
idt_r0 75,0
idt_r0 76,0
idt_r0 77,0
idt_r0 78,0
idt_r0 79,0
idt_r0 80,0