.global GenericInternalSyscall
.intel_syntax noprefix

.section .text

GenericInternalSyscall:
    mov r10, rcx            # Windows Kernel expects 1st arg in R10

    # We need to grab the SSN.
    # If we pass the SSN as the 12th argument, it will be at [rsp + 96]
    # (32 shadow + 8 ret addr + 7 overflow args * 8 bytes = 96)
    mov eax, [rsp + 96]

    syscall
    ret