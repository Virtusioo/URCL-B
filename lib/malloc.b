
/* STANDARD MALLOC.B */

/* Simple Bump Allocator */
/*
    FYI: Compiler by default always sets r25 as the heap base
*/

malloc __asm__ {
    "llod r2 sp 1" /* size */
    "mov r1 r25"
    "add r25 r25 r2"
}

mreset __asm__ {
    "imm r25 0"
}

msetbase __asm__ {
    "llod r25 sp 1" /* size */
}