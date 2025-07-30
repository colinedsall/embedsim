/*
A clock system that uses ticks to simulate a clock for any use case.
*/

#include "clock.hpp"

int NANOSECOND_SCALAR_VALUE = 1000000000;

Clock::Clock() : periodInNanoseconds(0), startPulseValue(false)
{
    cout << "Warning: Default constructor called. Clock will have no period. \n";
}

Clock::Clock(int periodInNanoseconds, bool startPulseValue) : 
        periodInNanoseconds(periodInNanoseconds),         // Timescale: nanoseconds
        startPulseValue(startPulseValue)                        // Initial clock value
{}

Clock::~Clock()
{
    stop();
}

void Clock::beginTicking(bool useSeconds)
{   
    if (running.load()) {
        cout << "Clock is already running. \n Stop the clock to restart it.";
        return;
    }

    running = true;
    clockOutput = startPulseValue;

    clockFuture = std::async(std::launch::async, &Clock::clockThreadLoop, this);

}

void Clock::stop()
{
    if (running.load()) {
        running = false;
    }

    if (clockFuture.valid()) {
        clockFuture.wait();
    }

    cout << "Clock stopped after " << clockCycles.load() << " cycles. \n";
}

void Clock::clockThreadLoop()
{
    int halfClockPeriod = periodInNanoseconds / 2;
    int clockWaitTime = 0;

    if (useSecondsMode) {
        clockWaitTime = halfClockPeriod * 1000000000;
    }
    else {
        clockWaitTime = halfClockPeriod;
    }

    cout << "Clock thread started. Half period: " << halfClockPeriod << " ns. \n";

    while (running.load()) {
        std::this_thread::sleep_for(nanoseconds(clockWaitTime));

        clockOutput =! clockOutput.load();

        clockCycles++;

        for (int i = 0; i < timers.size(); ++i) {
            timers[i].pollTimer();

            // Debug
            // cout << "Timer index: " << i << ". Current cycles: "
            //     << timers[i].getCurrentCycles() << " Rollover count: "
            //     << timers[i].getRolloverCount() << " \n";
        }

    }

    cout << "Clock thread exiting. \n";

}

bool Clock::createCountUpTimer(int timeInMilliseconds, bool outputRollovers)
{
    Timer timer(timeInMilliseconds, periodInNanoseconds, outputRollovers);
    cout << "Configured a new timer for " << timeInMilliseconds << " ms" 
        << " which " << (outputRollovers ? "does" : "does not") << "count rollovers.";

    timers.push_back(timer);

    // Return dummy true here
    return true;
}

void Clock::startCountUpTimer(int index)
{
    timers[index].startTimer();
}