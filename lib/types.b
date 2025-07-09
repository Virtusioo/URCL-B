
/* STANDARD TYPES.B */

iseven __asm__ {
    "llod r1 sp 1" /* number */
    "bev ~+3 r1"
    "imm r1 0"
    "ret"
    "imm r1 1"
}

isodd __asm__ {
    "llod r1 sp 1" /* number */
    "bod ~+3 r1"
    "imm r1 0"
    "ret"
    "imm r1 1"
}

isalpha __asm__ {
    "llod r2 sp 1" /* character */
    "setge r3 r2 'A'"
    "setle r4 r2 'Z'"
    "and r1 r3 r4"
    "setge r3 r2 'a'"
    "setle r4 r2 'z'"
    "and r3 r3 r4"
    "or r1 r1 r3"
}

isdigit __asm__ {
    "llod r2 sp 1" /* character */
    "setge r3 r2 '0'"
    "setle r4 r2 '9'"
    "and r1 r3 r4"  
}