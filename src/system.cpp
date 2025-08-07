#include "system.hpp"
#include <iostream>
#include <string>
#include <sstream>

// Constructors
System::System() : clock(10e4, false) {
    std::cout << "DEBUG: System constructor started" << std::endl;
    
    // Create QApplication first
    int argc = 1;
    char* argv[] = {(char*)"embedsim"};
    std::cout << "DEBUG: About to create QApplication" << std::endl;
    this->qtApp = std::make_unique<QApplication>(argc, argv);
    std::cout << "DEBUG: QApplication created successfully" << std::endl;
    
    // Then create Display
    std::cout << "DEBUG: About to initialize display" << std::endl;
    this->initializeDisplay();
    std::cout << "DEBUG: Display initialized successfully" << std::endl;
    
    std::cout << "DEBUG: System constructor completed" << std::endl;
}

System::~System() {
    std::cout << "DEBUG: System destructor started" << std::endl;
    
    // Stop CLI thread first
    stopCLIThread();
    std::cout << "DEBUG: CLI thread stopped" << std::endl;
    
    // Clean up Qt resources
    if (display) {
        std::cout << "DEBUG: Closing display" << std::endl;
        display->close();
        display.reset();
        std::cout << "DEBUG: Display closed and reset" << std::endl;
    }
    
    if (qtApp) {
        std::cout << "DEBUG: Quitting Qt application" << std::endl;
        qtApp->quit();
        qtApp.reset();
        std::cout << "DEBUG: Qt application quit and reset" << std::endl;
    }
    
    // Clear interrupt handlers to prevent dangling references
    interruptHandlers.clear();
    std::cout << "DEBUG: Interrupt handlers cleared" << std::endl;
    
    std::cout << "DEBUG: System destructor completed" << std::endl;
}   

void System::configureIO(IO io)
{
    this->io = io;
}

void System::registerInterrupt(const std::string& name, std::function<void()> handler)
{
    interruptHandlers[name] = handler;
}

void System::triggerInterrupt(const std::string& name)
{
    auto it = interruptHandlers.find(name);
    if (it != interruptHandlers.end()) {
        it->second();
    }
}

void System::stopClock()
{
    std::cout << "Interrupt: Stopping clock\n";
    clock.stop();
    shouldStop = true;
}

void System::startClock()
{
    std::cout << "Interrupt: Starting clock\n";
    clock.beginTicking(false);
    shouldStop = false;
}

void System::pauseClock()
{
    std::cout << "Interrupt: Pausing clock\n";
    clockPaused = true;
}

void System::resumeClock()
{
    std::cout << "Interrupt: Resuming clock\n";
    clockPaused = false;
}

void System::setupInterruptHandlers()
{
    // Register interrupt handlers for clock control
    registerInterrupt("stop_clock", [this]() {
        stopClock();
    });
    
    registerInterrupt("start_clock", [this]() {
        startClock();
    });
    
    registerInterrupt("pause_clock", [this]() {
        pauseClock();
    });
    
    registerInterrupt("resume_clock", [this]() {
        resumeClock();
    });
    
    registerInterrupt("user_stop", [this]() {
        std::cout << "Interrupt: User requested stop\n";
        shouldStop = true;
    });
    
    registerInterrupt("toggle_flag", [this]() {
        std::cout << "Interrupt: Toggling global flag\n";
        globalInterruptFlag = !globalInterruptFlag;
    });
}

void System::startCLIThread()
{
    if (!cliThreadRunning.load()) {
        cliThreadRunning = true;
        cliThread = std::thread(&System::cliInputLoop, this);
    }
}

void System::stopCLIThread()
{
    if (cliThreadRunning.load()) {
        cliThreadRunning = false;
        if (cliThread.joinable()) {
            cliThread.join();
        }
    }
}

void System::cliInputLoop()
{
    std::string input;
    while (cliThreadRunning.load()) {
        std::cout << "CLI> ";
        if (std::getline(std::cin, input)) {
            handleUserInput(input);
        }
    }
}

void System::handleUserInput(const std::string& input)
{
    std::lock_guard<std::mutex> lock(systemMutex);
    
    std::istringstream iss(input);
    std::string command;
    iss >> command;
    
    if (command == "stop") {
        triggerInterrupt("stop_clock");
    }
    else if (command == "start") {
        triggerInterrupt("start_clock");
    }
    else if (command == "pause") {
        triggerInterrupt("pause_clock");
    }
    else if (command == "resume") {
        triggerInterrupt("resume_clock");
    }
    else if (command == "flag") {
        triggerInterrupt("toggle_flag");
    }
    else if (command == "press") {
        std::string buttonName;
        iss >> buttonName;
        if (!buttonName.empty()) {
            io.setButtonPressed(buttonName, true);
            std::cout << "Simulated button press for: " << buttonName << "\n";
        }
    }
    else if (command == "release") {
        std::string buttonName;
        iss >> buttonName;
        if (!buttonName.empty()) {
            io.setButtonPressed(buttonName, false);
            std::cout << "Simulated button release for: " << buttonName << "\n";
        }
    }
    else if (command == "reset") {
        std::string buttonName;
        iss >> buttonName;
        if (!buttonName.empty()) {
            io.resetButton(buttonName);
            std::cout << "Reset button: " << buttonName << "\n";
        }
    }
    else if (command == "status") {
        std::cout << "Clock running: " << (clock.isRunning() ? "YES" : "NO") << "\n";
        std::cout << "Clock paused: " << (clockPaused.load() ? "YES" : "NO") << "\n";
        std::cout << "Global flag: " << (globalInterruptFlag.load() ? "ON" : "OFF") << "\n";
        std::cout << "Clock cycles: " << clock.getClockCycles() << "\n";
        
        // Show button states
        for (const auto& button : io.getButtons()) {
            std::cout << "Button " << button.name << ": ";
            std::cout << "Input=" << (button.inputState ? "HIGH" : "LOW") << ", ";
            std::cout << "State=";
            switch (button.state) {
                case ButtonState::IDLE: std::cout << "IDLE"; break;
                case ButtonState::DEBOUNCE: std::cout << "DEBOUNCE"; break;
                case ButtonState::PRESSED: std::cout << "PRESSED"; break;
                case ButtonState::RELEASED: std::cout << "RELEASED"; break;
            }
            std::cout << "\n";
        }
    }
    else if (command == "help") {
        std::cout << "Available commands:\n";
        std::cout << "  stop - Stop the clock and system\n";
        std::cout << "  start - Start the clock\n";
        std::cout << "  pause - Pause the clock (maintains state)\n";
        std::cout << "  resume - Resume the clock\n";
        std::cout << "  flag - Toggle global interrupt flag\n";
        std::cout << "  press <name> - Simulate button press\n";
        std::cout << "  release <name> - Simulate button release\n";
        std::cout << "  reset <name> - Reset button to IDLE state\n";
        std::cout << "  status - Show system status\n";
        std::cout << "  exit - Exit the program\n";
        std::cout << "  help - Show this help\n";
    }
    else if (command == "exit") {
        std::cout << "Exiting program...\n";
        shouldStop = true;
        
        // Clean up resources before exit
        if (display) {
            display->close();
        }
        
        stopCLIThread();
        
        if (qtApp) {
            qtApp->quit();
        }
        
        // Force cleanup
        std::cout << "Cleanup complete. Exiting.\n";
        std::exit(0);
    }
    else {
        std::cout << "Unknown command. Type 'help' for available commands.\n";
    }
}

void System::initializeDisplay()
{
    std::cout << "DEBUG: initializeDisplay started" << std::endl;
    
    // Ensure QApplication is properly initialized
    if (!QApplication::instance()) {
        std::cerr << "ERROR: QApplication not initialized!" << std::endl;
        return;
    }
    
    try {
        std::cout << "DEBUG: About to create DisplayApp" << std::endl;
        this->display = std::make_unique<DisplayApp>(400, 400);
        std::cout << "DEBUG: DisplayApp created successfully" << std::endl;
        
        // Connect the circle button click to system handler
        std::cout << "DEBUG: About to connect button click" << std::endl;
        this->display->connectButtonClick([this]() {
            this->handleCircleButtonClick();
        });
        std::cout << "DEBUG: Button click connected successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Exception in initializeDisplay: " << e.what() << std::endl;
        throw;
    } catch (...) {
        std::cerr << "ERROR: Unknown exception in initializeDisplay" << std::endl;
        throw;
    }
    
    std::cout << "DEBUG: initializeDisplay completed successfully" << std::endl;
}

void System::showText(const QString& text)
{
    if (this->display) {
        this->display->showWindow(text);
    }
}

void System::handleCircleButtonClick()
{
    std::cout << "Circle button clicked in GUI!\n";

    io.setButtonPressed("guiButton", true);
}

void System::handleButtonPress()
{
    static bool firstPress = false;
    if (!firstPress) {
        std::cout << "Button recognized as pressed\n";
        firstPress = true;
    }
}

void System::closeDisplay()
{
    if (this->display) {
        this->display->close();
    }
}


// Main function
void System::run()
{
    std::cout << "DEBUG: Starting System::run()" << std::endl;
    
    setupInterruptHandlers();
    std::cout << "DEBUG: Interrupt handlers setup complete" << std::endl;
    
    // Configure IO module
    IO anIO("SystemIO", true);
    anIO.addButton(Button("aButton"));
    this->configureIO(anIO);
    std::cout << "DEBUG: IO configuration complete" << std::endl;
    
    // Configure clock module
    clock.createCountUpTimer(1000, true);
    clock.beginTicking(false);
    clock.startCountUpTimer(0);
    std::cout << "DEBUG: Clock configuration complete" << std::endl;
    
    startCLIThread();
    std::cout << "DEBUG: CLI thread started" << std::endl;
    
    // Create timer for periodic operations instead of polling loop
    QTimer* systemTimer = new QTimer();
    std::cout << "DEBUG: QTimer created" << std::endl;
    
    QObject::connect(systemTimer, &QTimer::timeout, [this]() {
        std::lock_guard<std::mutex> lock(systemMutex);
        
        if (clock.isRunning() && !shouldStop.load()) {
            if (clock.getCurrentClockState() && !clockPaused.load()) {
                this->io.pollButtonsWithStates();
                
                // Handle button press logic
                if (this->io.isButtonPressed("aButton")) {
                    handleButtonPress();
                }
                
                // Check interrupt flags
                if (globalInterruptFlag.load()) {
                    std::cout << "Global flag is ON - performing special action\n";
                }
            }
        } else if (shouldStop.load()) {
            // Stop the Qt application
            QApplication::quit();
        }
    });
    
    // Start timer with appropriate interval (adjust as needed)
    systemTimer->start(10); // 10ms interval
    std::cout << "DEBUG: QTimer started" << std::endl;
    
    // Connect button handler
    this->display->connectButtonClick([this]() {
        std::lock_guard<std::mutex> lock(systemMutex);
        this->handleCircleButtonClick();
    });
    std::cout << "DEBUG: Button click connected" << std::endl;
    
    std::cout << "System started. Type 'help' for available commands.\n";
    
    // Show the display immediately after event loop starts
    QTimer* showTimer = new QTimer();
    showTimer->setSingleShot(true);
    QObject::connect(showTimer, &QTimer::timeout, [this]() {
        std::cout << "DEBUG: About to show display (delayed)" << std::endl;
        if (this->display) {
            this->display->showWindow("Embedded System");
            std::cout << "DEBUG: Display shown successfully" << std::endl;
        } else {
            std::cerr << "ERROR: Display is null!" << std::endl;
        }
    });
    showTimer->start(100); // Back to 100ms, but we'll also show it immediately
    
    // Also show the window immediately
    if (this->display) {
        std::cout << "DEBUG: Showing display immediately" << std::endl;
        this->display->showWindow("Embedded System");
    }
    
    // Enter Qt event loop - this will now handle everything
    std::cout << "DEBUG: About to enter Qt event loop" << std::endl;
    int result = this->qtApp->exec();
    std::cout << "DEBUG: Qt event loop exited with result: " << result << std::endl;
    
    // Clean up timers
    if (systemTimer) {
        systemTimer->stop();
        systemTimer->deleteLater();
    }
    if (showTimer) {
        showTimer->stop();
        showTimer->deleteLater();
    }
    
    std::cout << "System stopped.\n";
    stopCLIThread();
}