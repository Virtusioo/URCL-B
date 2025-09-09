
callback(n) {
    putnumb(n);
    putline();
}

countdown(n, callback) {
    if (n == 0)
        return;
    callback(n);
    countdown(n - 1, callback);
}

main() {
    countdown(5, callback);
}
