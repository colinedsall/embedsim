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
    
    // Set application icon early (affects dock and system menus)
    QIcon appIcon(":/icons/app_icon.png");
    if (!appIcon.isNull()) {
        QApplication::setWindowIcon(appIcon);
        std::cout << "DEBUG: Application icon set at QApplication level" << std::endl;
    } else {
        std::cout << "DEBUG: Could not load application icon at QApplication level" << std::endl;
    }
    
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
    else if (command == "close") {
        std::cout << "Closing display window...\n";
        
        if (display) {
            display->close();
            std::cout << "Display window closed.\n";
        } else {
            std::cout << "Error: No display to close.\n";
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
        std::cout << "  close - Close the display window\n";
        std::cout << "  exit - Exit the program\n";
        std::cout << "  help - Show this help\n";
        std::cout << "\nGraphics Commands:\n";
        std::cout << "  line <x1> <y1> <x2> <y2> <color> - Draw a line\n";
        std::cout << "  rect <x> <y> <width> <height> <color> - Draw a rectangle\n";
        std::cout << "  circle <x> <y> <radius> <color> - Draw a circle\n";
        std::cout << "  remove <id> - Remove graphics object by ID\n";
        std::cout << "  clear - Clear all graphics\n";
        std::cout << "  graphics - Show graphics objects info\n";
        std::cout << "  memory - Show graphics memory usage\n";
        std::cout << "  Note: Colors use hex format (e.g., FF0000 for red)\n";
    }
    else if (command == "line") {
        int x1, y1, x2, y2;
        std::string colorHex;
        if (iss >> x1 >> y1 >> x2 >> y2 >> colorHex) {
            std::cout << "Graphics command 'line' only available in display mode\n";
        } else {
            std::cout << "Usage: line <x1> <y1> <x2> <y2> <color>\n";
        }
    }
    else if (command == "rect") {
        int x, y, width, height;
        std::string colorHex;
        if (iss >> x >> y >> width >> height >> colorHex) {
            std::cout << "Graphics command 'rect' only available in display mode\n";
        } else {
            std::cout << "Usage: rect <x> <y> <width> <height> <color>\n";
        }
    }
    else if (command == "circle") {
        int x, y, radius;
        std::string colorHex;
        if (iss >> x >> y >> radius >> colorHex) {
            std::cout << "Graphics command 'circle' only available in display mode\n";
        } else {
            std::cout << "Usage: circle <x> <y> <radius> <color>\n";
        }
    }
    else if (command == "remove") {
        int id;
        if (iss >> id) {
            std::cout << "Graphics command 'remove' only available in display mode\n";
        } else {
            std::cout << "Usage: remove <id>\n";
        }
    }
    else if (command == "clear") {
        std::cout << "Graphics command 'clear' only available in display mode\n";
    }
    else if (command == "graphics") {
        std::cout << "Graphics command 'graphics' only available in display mode\n";
    }
    else if (command == "memory") {
        std::cout << "Graphics command 'memory' only available in display mode\n";
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
        this->display = std::make_unique<DisplayApp>(800, 500);
        std::cout << "DEBUG: DisplayApp created successfully" << std::endl;
        
        // Connect the circle button click to system handler
        std::cout << "DEBUG: About to connect button click" << std::endl;
        this->display->connectButtonClick([this]() {
            this->handleCircleButtonClick();
        });
        std::cout << "DEBUG: Button click connected successfully" << std::endl;
        
        // Setup timer callbacks
        setupTimerCallbacks();
        std::cout << "DEBUG: Timer callbacks setup completed" << std::endl;
        
        // Connect terminal command callback
        this->display->connectTerminalCommand([this](const std::string& command) {
            this->handleTerminalCommand(command);
        });
        std::cout << "DEBUG: Terminal command callback connected successfully" << std::endl;
        
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

void System::handleTerminalCommand(const std::string& command)
{
    // Process the command through the display-enabled CLI system
    handleUserInputWithDisplay(command);
}

void System::handleUserInputWithDisplay(const std::string& input)
{
    std::lock_guard<std::mutex> lock(systemMutex);
    
    std::istringstream iss(input);
    std::string command;
    iss >> command;
    
    // Capture output and send to display
    auto sendToDisplay = [this](const std::string& message) {
        if (display) {
            display->appendTerminalOutput(QString::fromStdString(message));
        }
        std::cout << message << std::endl; // Also keep console output
    };
    
    if (command == "stop") {
        triggerInterrupt("stop_clock");
        sendToDisplay("Clock stopped");
    }
    else if (command == "start") {
        triggerInterrupt("start_clock");
        sendToDisplay("Clock started");
    }
    else if (command == "pause") {
        triggerInterrupt("pause_clock");
        sendToDisplay("Clock paused");
    }
    else if (command == "resume") {
        triggerInterrupt("resume_clock");
        sendToDisplay("Clock resumed");
    }
    else if (command == "flag") {
        triggerInterrupt("toggle_flag");
        sendToDisplay("Global flag toggled");
    }
    else if (command == "press") {
        std::string buttonName;
        iss >> buttonName;
        if (!buttonName.empty()) {
            io.setButtonPressed(buttonName, true);
            sendToDisplay("Simulated button press for: " + buttonName);
        } else {
            sendToDisplay("Error: Please specify button name");
        }
    }
    else if (command == "release") {
        std::string buttonName;
        iss >> buttonName;
        if (!buttonName.empty()) {
            io.setButtonPressed(buttonName, false);
            sendToDisplay("Simulated button release for: " + buttonName);
        } else {
            sendToDisplay("Error: Please specify button name");
        }
    }
    else if (command == "reset") {
        std::string buttonName;
        iss >> buttonName;
        if (!buttonName.empty()) {
            io.resetButton(buttonName);
            sendToDisplay("Reset button: " + buttonName);
        } else {
            sendToDisplay("Error: Please specify button name");
        }
    }
    else if (command == "status") {
        sendToDisplay("=== System Status ===");
        sendToDisplay("Clock running: " + std::string(clock.isRunning() ? "YES" : "NO"));
        sendToDisplay("Clock paused: " + std::string(clockPaused.load() ? "YES" : "NO"));
        sendToDisplay("Global flag: " + std::string(globalInterruptFlag.load() ? "ON" : "OFF"));
        sendToDisplay("Clock cycles: " + std::to_string(clock.getClockCycles()));
        
        // Show button states
        for (const auto& button : io.getButtons()) {
            std::string buttonStatus = "Button " + button.name + ": ";
            buttonStatus += "Input=" + std::string(button.inputState ? "HIGH" : "LOW") + ", ";
            buttonStatus += "State=";
            switch (button.state) {
                case ButtonState::IDLE: buttonStatus += "IDLE"; break;
                case ButtonState::DEBOUNCE: buttonStatus += "DEBOUNCE"; break;
                case ButtonState::PRESSED: buttonStatus += "PRESSED"; break;
                case ButtonState::RELEASED: buttonStatus += "RELEASED"; break;
            }
            sendToDisplay(buttonStatus);
        }
        sendToDisplay("===================");
    }
    else if (command == "close") {
        sendToDisplay("Closing display window...");
        
        if (display) {
            display->close();
            sendToDisplay("Display window closed.");
        } else {
            sendToDisplay("Error: No display to close.");
        }
    }
    else if (command == "help") {
        sendToDisplay("Available commands:");
        sendToDisplay("  stop - Stop the clock and system");
        sendToDisplay("  start - Start the clock");
        sendToDisplay("  pause - Pause the clock (maintains state)");
        sendToDisplay("  resume - Resume the clock");
        sendToDisplay("  flag - Toggle global interrupt flag");
        sendToDisplay("  press <name> - Simulate button press");
        sendToDisplay("  release <name> - Simulate button release");
        sendToDisplay("  reset <name> - Reset button to IDLE state");
        sendToDisplay("  status - Show system status");
        sendToDisplay("  close - Close the display window");
        sendToDisplay("  exit - Exit the program");
        sendToDisplay("  help - Show this help");
        sendToDisplay("");
        sendToDisplay("Graphics Commands:");
        sendToDisplay("  line <x1> <y1> <x2> <y2> <color> - Draw a line");
        sendToDisplay("  rect <x> <y> <width> <height> <color> [solid|hollow] - Draw a rectangle");
        sendToDisplay("  circle <x> <y> <radius> <color> [solid|hollow] - Draw a circle");
        sendToDisplay("  remove <id> - Remove graphics object by ID");
        sendToDisplay("  fillstyle <id> <solid|hollow> - Change object fill style");
        sendToDisplay("  clear - Clear all graphics");
        sendToDisplay("  graphics - Show graphics objects info");
        sendToDisplay("  memory - Show graphics memory usage");
        sendToDisplay("  Note: Colors use hex format (e.g., FF0000 for red)");
        sendToDisplay("  Note: Fill styles: 'solid' or 'hollow' (default: solid, case-insensitive)");
    }
    else if (command == "line") {
        int x1, y1, x2, y2;
        std::string colorHex;
        if (iss >> x1 >> y1 >> x2 >> y2 >> colorHex) {
            if (display) {
                int id = display->drawLine(x1, y1, x2, y2, QString::fromStdString(colorHex));
                if (id > 0) {
                    sendToDisplay("Line drawn with ID: " + std::to_string(id));
                } else {
                    sendToDisplay("Error: Failed to draw line");
                }
            }
        } else {
            sendToDisplay("Usage: line <x1> <y1> <x2> <y2> <color>");
        }
    }
    else if (command == "rect") {
        int x, y, width, height;
        std::string colorHex;
        std::string fillStyle;
        if (iss >> x >> y >> width >> height >> colorHex) {
            // Check if fill style is specified
            if (iss >> fillStyle) {
                bool solid = (fillStyle == "solid" || fillStyle == "s" || fillStyle == "Solid" || fillStyle == "S");
                if (display) {
                    int id = display->drawRectangle(x, y, width, height, QString::fromStdString(colorHex), solid);
                    if (id > 0) {
                        sendToDisplay("Rectangle drawn with ID: " + std::to_string(id) + " (" + (solid ? "solid" : "hollow") + ")");
                    } else {
                        sendToDisplay("Error: Failed to draw rectangle");
                    }
                }
            } else {
                // Default to solid
                if (display) {
                    int id = display->drawRectangle(x, y, width, height, QString::fromStdString(colorHex));
                    if (id > 0) {
                        sendToDisplay("Rectangle drawn with ID: " + std::to_string(id) + " (solid)");
                    } else {
                        sendToDisplay("Error: Failed to draw rectangle");
                    }
                }
            }
        } else {
            sendToDisplay("Usage: rect <x> <y> <width> <height> <color> [solid|hollow]");
        }
    }
    else if (command == "circle") {
        int x, y, radius;
        std::string colorHex;
        std::string fillStyle;
        if (iss >> x >> y >> radius >> colorHex) {
            // Check if fill style is specified
            if (iss >> fillStyle) {
                bool solid = (fillStyle == "solid" || fillStyle == "s" || fillStyle == "Solid" || fillStyle == "S");
                if (display) {
                    int id = display->drawCircle(x, y, radius, QString::fromStdString(colorHex), solid);
                    if (id > 0) {
                        sendToDisplay("Circle drawn with ID: " + std::to_string(id) + " (" + (solid ? "solid" : "hollow") + ")");
                    } else {
                        sendToDisplay("Error: Failed to draw circle");
                    }
                }
            } else {
                // Default to solid
                if (display) {
                    int id = display->drawCircle(x, y, radius, QString::fromStdString(colorHex));
                    if (id > 0) {
                        sendToDisplay("Circle drawn with ID: " + std::to_string(id) + " (solid)");
                    } else {
                        sendToDisplay("Error: Failed to draw circle");
                    }
                }
            }
        } else {
            sendToDisplay("Usage: circle <x> <y> <radius> <color> [solid|hollow]");
        }
    }
    else if (command == "remove") {
        int id;
        if (iss >> id) {
            if (display) {
                if (display->removeGraphicsObject(id)) {
                    sendToDisplay("Graphics object " + std::to_string(id) + " removed");
                } else {
                    sendToDisplay("Error: Object " + std::to_string(id) + " not found");
                }
            }
        } else {
            sendToDisplay("Usage: remove <id>");
        }
    }
    else if (command == "clear") {
        if (display) {
            display->clearGraphics();
            sendToDisplay("All graphics cleared");
        }
    }
    else if (command == "graphics") {
        if (display) {
            QString info = display->getGraphicsInfo();
            sendToDisplay("Graphics Objects:");
            sendToDisplay(info.toStdString());
        }
    }
    else if (command == "memory") {
        if (display) {
            size_t memory = display->getGraphicsMemoryUsage();
            sendToDisplay("Graphics Memory Usage: " + std::to_string(memory) + " bytes");
        }
    }
    else if (command == "fillstyle") {
        int id;
        std::string fillStyle;
        if (iss >> id >> fillStyle) {
            bool solid = (fillStyle == "solid" || fillStyle == "s" || fillStyle == "Solid" || fillStyle == "S");
            if (display) {
                display->setObjectFillStyle(id, solid);
                sendToDisplay("Object " + std::to_string(id) + " fill style changed to " + (solid ? "solid" : "hollow"));
            }
        } else {
            sendToDisplay("Usage: fillstyle <id> <solid|hollow>");
        }
    }
    else if (command == "exit") {
        sendToDisplay("Exiting program...");
        
        // Use the same approach as the close command - close the window first
        // This works and causes the Qt event loop to exit properly
        if (display) {
            display->close();
            sendToDisplay("Display window closed.");
        }
        
        // Exit the program immediately after closing the window
        sendToDisplay("Exit requested. Terminating program...");
        std::cout << "DEBUG: Exit command: Terminating program immediately" << std::endl;
        std::exit(0);
    }
    else if (!command.empty()) {
        sendToDisplay("Unknown command: " + command + " (type 'help' for available commands)");
    }
}

void System::closeDisplay()
{
    if (this->display) {
        this->display->close();
    }
}

void System::setupTimerCallbacks()
{
    if (!display) return;
    
    // Connect add timer callback
    display->connectAddTimerCallback([this](const std::string& name, int timeMs) {
        this->addTimer(name, timeMs);
    });
    
    // Connect start timer callback
    display->connectStartTimerCallback([this](const std::string& name) {
        this->startTimer(name);
    });
    
    // Connect stop timer callback
    display->connectStopTimerCallback([this](const std::string& name) {
        this->stopTimer(name);
    });
    
    // Connect remove timer callback
    display->connectRemoveTimerCallback([this](const std::string& name) {
        this->removeTimer(name);
    });
}

void System::addTimer(const std::string& name, int timeMs)
{
    std::lock_guard<std::mutex> lock(timerMutex);
    
    // Check if timer already exists
    for (const auto& managedTimer : managedTimers) {
        if (managedTimer.name == name) {
            std::cout << "Timer '" << name << "' already exists" << std::endl;
            return;
        }
    }
    
    // Create new timer with system clock period
    int systemClockPeriodNs = clock.getSystemClockPeriodInNanoseconds();
    auto timer = std::make_shared<Timer>(timeMs, systemClockPeriodNs, true);
    
    ManagedTimer managedTimer;
    managedTimer.name = name;
    managedTimer.timeMs = timeMs;
    managedTimer.timer = timer;
    managedTimer.isRunning = false;
    
    managedTimers.push_back(managedTimer);
    
    // Update display
    updateTimerDisplay();
    
    std::cout << "Added timer '" << name << "' with " << timeMs << "ms duration" << std::endl;
}

void System::startTimer(const std::string& name)
{
    std::lock_guard<std::mutex> lock(timerMutex);
    
    for (auto& managedTimer : managedTimers) {
        if (managedTimer.name == name) {
            if (managedTimer.timer) {
                managedTimer.timer->startTimer();
                managedTimer.isRunning = true;
                std::cout << "Started timer '" << name << "'" << std::endl;
            }
            break;
        }
    }
    
    updateTimerDisplay();
}

void System::stopTimer(const std::string& name)
{
    std::lock_guard<std::mutex> lock(timerMutex);
    
    for (auto& managedTimer : managedTimers) {
        if (managedTimer.name == name) {
            if (managedTimer.timer) {
                // Note: Timer class doesn't have a stop method, so we'll just mark it as not running
                managedTimer.isRunning = false;
                std::cout << "Stopped timer '" << name << "'" << std::endl;
            }
            break;
        }
    }
    
    updateTimerDisplay();
}

void System::removeTimer(const std::string& name)
{
    std::lock_guard<std::mutex> lock(timerMutex);
    
    managedTimers.erase(
        std::remove_if(managedTimers.begin(), managedTimers.end(),
            [&name](const ManagedTimer& timer) { return timer.name == name; }),
        managedTimers.end()
    );
    
    updateTimerDisplay();
    std::cout << "Removed timer '" << name << "'" << std::endl;
}

void System::updateTimerDisplay()
{
    if (!display) return;
    
    // Update clock cycles
    display->updateClockCycles(clock.getClockCycles());
    
    // Convert managed timers to display items
    std::vector<TimerDisplayItem> displayItems;
    
    for (const auto& managedTimer : managedTimers) {
        TimerDisplayItem item;
        item.name = managedTimer.name;
        item.timeMs = managedTimer.timeMs;
        item.timer = managedTimer.timer;
        item.isRunning = managedTimer.isRunning;
        
        if (managedTimer.timer) {
            item.currentCycles = managedTimer.timer->getCurrentCycles();
            item.rolloverCount = managedTimer.timer->getRolloverCount();
        } else {
            item.currentCycles = 0;
            item.rolloverCount = 0;
        }
        
        displayItems.push_back(item);
    }
    
    display->updateTimerStatus(displayItems);
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
                
                // Poll managed timers
                {
                    std::lock_guard<std::mutex> timerLock(timerMutex);
                    for (auto& managedTimer : managedTimers) {
                        if (managedTimer.isRunning && managedTimer.timer) {
                            managedTimer.timer->pollTimer();
                        }
                    }
                }
                
                // Update timer display periodically
                static int displayUpdateCounter = 0;
                if (++displayUpdateCounter >= 10) { // Update every 10 cycles
                    updateTimerDisplay();
                    displayUpdateCounter = 0;
                }
                
                // Check interrupt flags
                if (globalInterruptFlag.load()) {
                    std::cout << "Global flag is ON - performing special action\n";
                }
            }
        } else if (shouldStop.load()) {
            // Exit is now handled directly in the exit handler
            // This prevents hanging issues with Qt quit
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
    
    // Debug: Check the shouldStop flag
    std::cout << "DEBUG: shouldStop flag value: " << (shouldStop.load() ? "true" : "false") << std::endl;
    
    // If we're supposed to stop, exit the program
    if (shouldStop.load()) {
        std::cout << "Exit requested. Terminating program." << std::endl;
        std::exit(0);
    }
    
    // Let the program exit naturally after proper cleanup
    std::cout << "System cleanup complete. Exiting normally." << std::endl;
}