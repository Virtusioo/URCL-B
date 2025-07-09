
/* LIB2D DRAW.B */

DrawPixel __asm__ {
    "llod r1 sp 1" /* y */
    "llod r2 sp 2" /* x */
    "out %x r2"
    "out %y r1"
    "out %color r24"
}

DrawLine __asm__ {
    "llod r1 sp 1" /* y2 */
    "llod r2 sp 2" /* x2 */
    "llod r3 sp 3" /* y1 */
    "llod r4 sp 4" /* x1 */
    "out %x1 r4"
    "out %y1 r3"
    "out %x2 r2"
    "out %y2 r1"
    "out %line r24"
}

DrawRectangle __asm__ {
    "llod r1 sp 1" /* height */
    "llod r2 sp 2" /* width */
    "llod r3 sp 3" /* y */
    "llod r4 sp 4" /* x */
    "add r5 r3 r1"
    "add r6 r4 r2"
    "mov r7 r4"
    "out %x r7"
    "out %y r3"
    "out %color r24"
    "add r7 r7 1"
    "brl ~-4 r7 r6"
    "add r3 r3 1"
    "brl ~-7 r3 r5"
}