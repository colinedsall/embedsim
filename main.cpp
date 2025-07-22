#include <iostream>
#include "clock.hpp"
#include "io.hpp"

int main() {
    Clock aClock(10e6, false);
    aClock.beginTicking(false);

    aClock.createCountUpTimer(500, true);
    aClock.createCountUpTimer(1000, true);
    aClock.startCountUpTimer(0);
    aClock.startCountUpTimer(1);

    IOType type = {"aTimer", false};
    IO io(type);

    
    while (true) {
        if (aClock.isRunning()) {

            // std::this_thread::sleep_for(milliseconds(1501));

            int toggle = 0;
            cin >> toggle;

            if (toggle != 0) {
                aClock.stop();
            }
        }

        else {
            break;
        }
    }

    return 0;
}