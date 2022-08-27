[bits 32]
[GLOBAL switch_r0]
[GLOBAL to_schdule]
global switch_r0
global to_schdule
extern schdule
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

to_schdule:
    cli
    mov eax,[esp]
    pushfd
    push cs
    push eax    ;;eip
    mov eax,esp
    push 0      ;;eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    add eax,4*4
    sub eax,12*4
    push eax
    call schdule