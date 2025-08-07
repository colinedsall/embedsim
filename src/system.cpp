#include "system.hpp"
#include <iostream>
#include <string>
#include <sstream>

// Constructors
System::System() : clock(10e4, false) {
    // Create QApplication first
    int argc = 1;
    char* argv[] = {(char*)"embedsim"};
    this->qtApp = std::make_unique<QApplication>(argc, argv);
    
    // Then create Display
    this->initializeDisplay();
}

System::~System() {
    stopCLIThread();
    
    // Clean up Qt resources
    if (display) {
        display->close();
        display.reset();
    }
    
    if (qtApp) {
        qtApp->quit();
        qtApp.reset();
    }
    
    interruptHandlers.clear();
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
        std::cout << "  button <name> - Simulate button press\n";
        std::cout << "  release <name> - Simulate button release\n";
        std::cout << "  reset <name> - Reset button to IDLE state\n";
        std::cout << "  status - Show system status\n";
        std::cout << "  help - Show this help\n";
    }
    else if (command == "exit") {
        std::cout << "Exiting program...\n";
        shouldStop = true;
        
        if (display) {
            display->close();
        }
        
        stopCLIThread();
        
        if (qtApp) {
            qtApp->quit();
        }
        
        exit(0);
    }
    else {
        std::cout << "Unknown command. Type 'help' for available commands.\n";
    }
}

void System::initializeDisplay()
{
    this->display = std::make_unique<DisplayApp>(400, 400);
    
    // Connect the circle button click to system handler
    this->display->connectButtonClick([this]() {
        this->handleCircleButtonClick();
    });
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

void System::closeDisplay()
{
    if (this->display) {
        this->display->close();
    }
}


// Main function
void System::run()
{
    // Setup interrupt handlers
    setupInterruptHandlers();

    // Config before polling loop below
    // Configure IO module, ensuring it is enabled
    IO anIO("SystemIO", true);
    anIO.addButton(Button("aButton"));
    this->configureIO(anIO);

    // Configure clock module
    clock.createCountUpTimer(1000, true);
    clock.beginTicking(false);

    // Start timer based module
    clock.startCountUpTimer(0);
    
    // Start CLI thread for real-time input
    startCLIThread();

    std::cout << "System started. Type 'help' for available commands.\n";

    bool firstPress = false;
    this->display->showWindow("Embedded System");
    this->qtApp->exec();

    // Polling loop - synchronized with clock
    while (clock.isRunning() && !shouldStop.load()) {
        if (clock.getCurrentClockState() == true && !clockPaused.load()) {
            // Clock-synchronized operations
            this->io.pollButtonsWithStates(); // Use actual button input states

            // std::cout << "Counter: " << counter << "\n";
            // std::cout << "Pressed count: " << this->io.pressedCount << std::endl;
            // std::cout << "Button state: " << this->io.isButtonPressed("aButton") << std::endl;
            
            // Check for interrupt flags (atomic operations)
            if (globalInterruptFlag.load()) {
                std::cout << "Global flag is ON - performing special action\n";
            }

        }

        if (this->io.isButtonPressed("aButton") && firstPress == false) {
            std::cout << "Button recognized as pressed\n";
            // this->handleCircleButtonClick();
            
            firstPress = true;

        }

        else if (clockPaused.load()) {
            // Clock is paused, just wait
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        else {
            // Optional: Handle low clock state
        }
    }
    
    std::cout << "System stopped.\n";
    stopCLIThread();
}