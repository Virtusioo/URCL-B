
/* STANDARD MATH.B */

abs __asm__ {
    "llod r1 sp 1" /* integer */
    "abs r1 r1"
}

fsqrt __asm__ {
    "llod r1 sp 1" /* float */
    "fsqrt r1 r1"
}

itof __asm__ {
    "llod r1 sp 1" /* integer */
    "itof r1 r1"
}

ftoi __asm__ {
    "llod r1 sp 1" /* float */
    "ftoi r1 r1"
}

fadd __asm__ {
    "llod r1 sp 1" /* float */
    "llod r2 sp 2" /* float */ 
    "fadd r1 r1 r2"
}

fmul __asm__ {
    "llod r1 sp 1" /* float */
    "llod r2 sp 2" /* float */ 
    "fmul r1 r1 r2"
}

fdiv __asm__ {
    "llod r1 sp 1" /* float */
    "llod r2 sp 2" /* float */ 
    "fdiv r1 r1 r2"
}