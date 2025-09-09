
callback __asm__ {
    "llod r1 sp 1"
    "out %numb r1"
    "out %text 10"
}

call(n, callback) {
    callback(n);
}

main() {
    call(69, callback);
}
