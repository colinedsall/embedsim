#include "timer.hpp"

Timer::Timer() : clockCycles(0), systemClockPeriodInNanoseconds(0)
{
    cout << "Warning: Default constructor called, timer clock cycles not initialized (0). \n";
}

Timer::Timer(int milliseconds, int systemClockPeriodInNanoseconds, bool continuousRun) : 
    systemClockPeriodInNanoseconds(systemClockPeriodInNanoseconds), continuousRun(continuousRun)
{
    // Using the system clock period, configure the timer to run
    // for the amount of time configured

    // Cycles (#) = time (s) / period (s)
    const int NS_PER_MS = 1'000'000;
    clockCycles = (milliseconds * NS_PER_MS) / systemClockPeriodInNanoseconds;

}

Timer::~Timer() {}

bool Timer::isRunning()
{
    return running;
}

void Timer::startTimer()
{
    running = true;
}

void Timer::pollTimer()
{
    if (running) {
        if (continuousRun && hasRolledOver) {
            hasRolledOver = false;
        }

        if (currentCycles == clockCycles && !hasRolledOver) {
            hasRolledOver = true;
            rolloverCount++;
            currentCycles = 0;
        }

        else {
            currentCycles++;
        }
    }

    else {
        // Do nothing
    }
}

int Timer::getCurrentCycles()
{
    return currentCycles;
}

int Timer::getRolloverCount()
{
    return rolloverCount;
}