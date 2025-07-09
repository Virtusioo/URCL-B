
main() {
    Init2D();
    x = 0;
    while (SyncFrame()) {
        ClearScreen();
        x = x + 1;
        DrawRectangle(x, x, 10, 10);
        FlipScreen();
    }
}