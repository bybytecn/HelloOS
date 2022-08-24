[bits 32]
[GLOBAL switch_r0]
global switch_r0
; ss, gs, es, fs, ds, ebp, edi, esi, edx, ecx, ebx, eax, esp
switch_r0:
    add esp,4
    pop ss
    pop gs
    pop es
    pop fs
    pop ds
    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    pop esp
    sub esp,3*4
    iret