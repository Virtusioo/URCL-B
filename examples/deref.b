
assign(x, value) {
    *x = value;
}

main() {
    y = 420;
    assign(&y, 69);
    putnumb(y);
}