

main() {
    auto y, vy;
    x = 55;

    Init2D();
    SetTargetFPS(10);

    while (SyncFrame()) {
        ClearScreen();
        vy = vy + 1;
        y = y + vy;

        if (y + 10 >= 96) {
            vy = vy * 65535;
            y = 86; 
        }
        DrawRectangle(x, y, 10, 10);
        FlipScreen();
    }
}