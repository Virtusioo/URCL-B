
log(str) {
    while (*str) {
        putchar(*str);
        str = str + 1;
    }
}

main() {
    log("Hello, World!");
}