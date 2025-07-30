#include <iostream>

using namespace std;

#ifndef TIMER_HPP
#define TIMER_HPP

class Timer
{
    public:
        Timer();
        Timer(int milliseconds, int systemClockPeriodInNanoseconds, bool continuousRun);
        ~Timer();

        void startTimer();
        bool isRunning();
        void pollTimer();

        int getCurrentCycles();
        int getRolloverCount();

    private:
        int systemClockPeriodInNanoseconds = 0;
        long long int clockCycles = 0;
        int currentCycles = 0;
        int rolloverCount = 0;

        bool running = false;
        bool hasRolledOver = false;
        bool continuousRun = false;
};

#endif