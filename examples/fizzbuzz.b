

fizzbuzz(n) {
    i = 1;
    while (i < n) {
        if (i % 15 == 0) {
            puts("FizzBuzz");
        } else if (i % 3 == 0) {
            puts("Fizz");
        } else if (i % 5 == 0) {
            puts("Buzz");
        } else {
            putnumb(i);
            putline();
        }
        i = i + 1;
    }
}

main() {
    fizzbuzz(15);
}