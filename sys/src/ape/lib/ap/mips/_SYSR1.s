TEXT _SYSR1(SB), 1, $0
MOVW R1, 0(FP)
MOVW $0, R1
SYSCALL
RET
