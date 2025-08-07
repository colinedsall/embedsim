#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <functional>
#include <map>
#include "clock.hpp"
#include "io.hpp"
#include "display.hpp"
#include <QApplication>

class System {
public:
    System();
    ~System();
    
    void configureIO(IO io);
    void run();
    
    // Interrupt-like functionality
    void registerInterrupt(const std::string& name, std::function<void()> handler);
    void triggerInterrupt(const std::string& name);
    void startCLIThread();
    void stopCLIThread();
    
    // Direct clock control through interrupts
    void stopClock();
    void startClock();
    void pauseClock();
    void resumeClock();

    // Display control methods
    void initializeDisplay();
    void showText(const QString& text);
    void closeDisplay();
    void handleCircleButtonClick();
    
    // Global state that can be modified by interrupts
    std::atomic<bool> globalInterruptFlag{false};
    std::atomic<bool> shouldStop{false};
    std::atomic<bool> clockPaused{false};
    std::atomic<int> userCommand{0};

private:
    Clock clock;
    IO io;
    std::unique_ptr<DisplayApp> display;
    std::unique_ptr<QApplication> qtApp;
    
    // Interrupt system
    std::map<std::string, std::function<void()>> interruptHandlers;
    std::thread cliThread;
    std::atomic<bool> cliThreadRunning{false};
    
    void cliInputLoop();
    void handleUserInput(const std::string& input);
    void setupInterruptHandlers();
};

#endif