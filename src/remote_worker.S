.intel_syntax noprefix
.text

// Worker IL2CPP phục vụ tối đa bốn tham số theo Win64 ABI.
.global RemoteWorkerStart
.global RemoteWorkerEnd

RemoteWorkerStart:
    push rbx
    push rsi
    push rdi
    push r12
    sub rsp, 0x28
    mov rbx, rcx

    mov rax, qword ptr [rbx + 56]
    call rax
    test rax, rax
    je RemoteWorkerExit
    mov rcx, rax
    mov rax, qword ptr [rbx + 64]
    call rax
    test rax, rax
    je RemoteWorkerExit
    mov r12, rax

RemoteWorkerLoop:
    // Offset 4 is a dedicated stop flag. A long game call only clears the
    // command at offset 0, so it can no longer erase a pending stop request.
    mov eax, dword ptr [rbx + 4]
    test eax, eax
    jne RemoteWorkerStop
    mov eax, dword ptr [rbx]
    cmp eax, 1
    je RemoteWorkerCall
    mov ecx, 1
    mov rax, qword ptr [rbx + 80]
    call rax
    jmp RemoteWorkerLoop

RemoteWorkerCall:
    mov rax, qword ptr [rbx + 8]
    mov rcx, qword ptr [rbx + 16]
    mov rdx, qword ptr [rbx + 24]
    mov r8,  qword ptr [rbx + 32]
    mov r9,  qword ptr [rbx + 40]
    call rax
    mov qword ptr [rbx + 48], rax
    mov dword ptr [rbx], 0
    jmp RemoteWorkerLoop

RemoteWorkerStop:
    mov rcx, r12
    mov rax, qword ptr [rbx + 72]
    call rax

RemoteWorkerExit:
    xor eax, eax
    add rsp, 0x28
    pop r12
    pop rdi
    pop rsi
    pop rbx
    ret

RemoteWorkerEnd:
