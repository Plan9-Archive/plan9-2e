/*
 * Test and set for the 68020, as a subroutine
 */

TEXT	_tas(SB), $0

	MOVL	$0, R0
	MOVL	keyaddr+0(FP), A0
	TAS	(A0)
	BEQ	tas_1
	MOVL	$1, R0
tas_1:
	RTS
