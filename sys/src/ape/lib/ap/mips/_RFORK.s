TEXT _RFORK(SB), 1, $0
MOVW R1, 0(FP)
MOVW $19, R1
SYSCALL
RET
