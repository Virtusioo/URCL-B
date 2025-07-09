
/* LIB2D FRAME.B */

SetTargetFPS __asm__ {
    "llod r1 sp 1" /* fps */
    "div r23 1000 r1"
}

SetTargetDelay __asm__ {
    "llod r23 sp 1" /* ms */
}

SyncFrame __asm__ {
    "imm r1 1"
    "out %buffer 1"
    "out %wait r23"
    "in r0 %wait"
}

FlushFrame __asm__ {
    "imm r1 1"
    "out %buffer 1"
}

SetColor __asm__ {
    "llod r24 sp 1" /* color */
}

ClearScreen __asm__ {
    "out %buffer 0"
    "out %buffer 1"
}

FlipScreen __asm__ {
    "out %buffer 2"
}