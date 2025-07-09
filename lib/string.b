
/* STANDARD STRING.B */

strlen __asm__ {
    "llod r2 sp 1" /* string ptr */
    "imm r1 0"
    "lod r3 r2"
    "bre ~+4 r3 0"
    "inc r1 r1"
    "inc r2 r2"
    "jmp ~-4"
}

strnlen __asm__ {
    "llod r2 sp 1"     /* max N */
    "llod r3 sp 2"     /* string pointer */
    "imm r1 0" 
    "bre ~+6 r1 r2"    
    "lod r4 r3"     
    "bre ~+4 r4 0"
    "inc r1 r1"  
    "inc r3 r3"
    "jmp ~-5" 
}

memcpy __asm__ {
    "llod r2 sp 1" /* size */
    "llod r3 sp 2" /* source */
    "llod r1 sp 3" /* dest */
    "mov r4 r1" /* copy dest */
    "brz ~+7 r2"
    "lod r5 r3"
    "str r4 r5"
    "inc r3 r3"
    "inc r4 r4"
    "dec r2 r2"
    "jmp ~-6"
    "nop"
}