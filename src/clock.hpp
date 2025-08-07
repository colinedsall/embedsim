#include <iostream>
#include <ctime>
#include <chrono>
#include <thread>
#include <atomic>
#include <future>
#include <vector>

#include "timer.hpp"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

// A clock system that uses ticks to simulate a clock for any use case.

#ifndef CLOCK_HPP
#define CLOCK_HPP

class Clock
{
    public:
        Clock();
        Clock(int periodInNanoseconds, bool startPulseValue);
        ~Clock();

        void beginTicking(bool useSeconds);
        void stop();
        bool getCurrentClockState() const {return clockOutput.load();}
        long long getClockCycles() const {return clockCycles.load(); }
        bool isRunning() const {return running.load(); }

        bool createCountUpTimer(int timeInMilliseconds, bool outputRollovers);
        void startCountUpTimer(int index);
    /*
    Initialize a clock with a specific frequency of operation. This serves
    as the basis for any scheduled events.
    */

    private:
        // Using threaded-based variables instead of class-based, single-thread
        // This should allow us to access other lines of code while the clock runs
        std::atomic<bool> clockOutput{false};
        std::atomic<long long> clockCycles{0};
        std::atomic<bool> running{false};
        std::future<void> clockFuture;

        // Clock configuration
        int periodInNanoseconds = 0;
        bool startPulseValue = 0;
        bool useSecondsMode = false;

        // Utility members
        vector<Timer> timers;
        
        void clockThreadLoop();

};

#endif