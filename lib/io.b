
/* STANDARD IO.B */

/* OUTPUT BEGIN */

putchar __asm__ {
    "llod r1 sp 1" /* character */
    "out %text r1"
}

putnumb __asm__ { 
    "llod r1 sp 1" /* number */
    "out %numb r1" 
}

putint __asm__ {
    "llod r1 sp 1" /* integer */
    "out %int r1"
}

putfloat __asm__ {
    "llod r1 sp 1" /* float */
    "out %float r1"
}

puts __asm__ {
    "llod r1 sp 1" /* string pointer */
    "lod r2 r1"
    "bre ~+4 r2 0"
    "out %text r2"
    "inc r1 r1"
    "jmp ~-4"
    "out %text 10"
}

print __asm__ {
    "llod r1 sp 1" /* string pointer */
    "lod r2 r1"
    "bre ~+4 r2 0"
    "out %text r2"
    "inc r1 r1"
    "jmp ~-4"
}

/* OUTPUT END */

/* INPUT BEGIN */

getnumb __asm__ {
    "in r1 %numb"
}

getchar __asm__ {
    "in r1 %text"
}

/* INPUT END */

delay __asm__ {
    "llod r1 sp 1" /* ms */
    "out %wait r1"
    "in r0 %wait"
}

/* SOUND IO BEGIN */

tone __asm__ {
    "llod r1 sp 1" /* tone length */
    "llod r2 sp 2" /* tone */
    "out %note r2"
    "out %nleg r1"
}

dtone __asm__ {
    "llod r1 sp 1" /* delay */
    "llod r2 sp 2" /* tone length */
    "llod r3 sp 3" /* tone */
    "out %note r3"
    "out %nleg r2"
    "out %wait r1"
    "in r0 %wait"
}

/* SOUND IO END */
