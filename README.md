# Embedded System Simulator

A C++ simulation framework for embedded systems with clock, timer, I/O, and interrupt-like functionality.

## Features

- **Clock System**: Configurable clock with nanosecond precision
- **Timer Module**: Count-up timers with rollover detection
- **I/O System**: Button simulation with FSM-based debouncing
- **Interrupt-like CLI**: Real-time user input handling via separate thread
- **Direct Clock Control**: Stop, start, pause, and resume clock via CLI
- **Thread-Safe**: Atomic operations for thread communication

## Usage

### Running the System
```bash
mkdir build && cd build
cmake ..
make
./embedsim
```

### CLI Commands (Real-time Input)
While the system is running, you can use these commands in the CLI prompt:

#### Clock Control
- `stop` - Stop the clock and system (interrupt)
- `start` - Start the clock (interrupt)
- `pause` - Pause the clock while maintaining state (interrupt)
- `resume` - Resume the clock (interrupt)

#### System Control
- `flag` - Toggle global interrupt flag
- `press <name>` - Simulate button press
- `release <name>` - Simulate button release
- `reset <name>` - Reset button to IDLE state
- `status` - Show system status (clock state, cycles, flags, button states)
- `help` - Show available commands

### Example Session
```
CLI> status
Clock running: YES
Clock paused: NO
Global flag: OFF
Clock cycles: 0
press aButton: Input=LOW, State=IDLE

CLI> press aButton
Simulated button press for: aButton

CLI> status
Clock running: YES
Clock paused: NO
Global flag: OFF
Clock cycles: 15
Button aButton: Input=HIGH, State=PRESSED

CLI> release aButton
Simulated button release for: aButton

CLI> status
Clock running: YES
Clock paused: NO
Global flag: OFF
Clock cycles: 30
Button aButton: Input=LOW, State=RELEASED

CLI> reset aButton
Reset button: aButton

CLI> stop
Interrupt: Stopping clock
System stopped.
```

## Architecture

### Clock-Synchronized Operations
- Main polling loop runs synchronized with clock cycles
- I/O polling and timer operations occur on positive clock edges
- Atomic flags allow real-time communication between threads

### Interrupt-like System
- CLI runs in separate thread for real-time input
- Atomic variables provide thread-safe communication
- Interrupt handlers can be registered for custom actions
- Direct clock control through interrupt triggers

### Clock Control Flow
1. **User Input**: CLI command entered
2. **Interrupt Trigger**: `triggerInterrupt()` called
3. **Handler Execution**: Registered interrupt handler executes
4. **Clock Control**: Direct clock methods called (`stop()`, `pause()`, etc.)
5. **State Update**: Atomic flags updated for thread communication

## Key Components

- **System**: Main orchestrator with interrupt handling and clock control
- **Clock**: Threaded clock with configurable frequency and direct control
- **IO**: Button simulation with FSM debouncing
- **Timer**: Count-up timers with rollover detection

# Component Explanations

## Display Module

A Qt-based display module for the embedded system simulator that creates graphical windows.

## Features

- **400x400 white window** by default
- **Centered Arial text** display
- **Customizable size** and text content
- **Qt integration** for cross-platform GUI

## Requirements

- Qt5 or Qt6 installed on your system
- CMake 3.13 or higher

## Installation

### Installing Qt

**macOS:**
```bash
brew install qt
```

**Ubuntu/Debian:**
```bash
sudo apt-get install qt6-base-dev
# or for Qt5:
sudo apt-get install qt5-default
```

**Windows:**
Download Qt from https://www.qt.io/download

## Usage

### Basic Usage

```cpp
#include <QApplication>
#include "display.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    Display display(400, 400);
    display.showWindow("Hello World!");
    
    return app.exec();
}
```

### Integration with System

```cpp
#include "system_display.hpp"

int main()
{
    SystemWithDisplay system;
    
    // Initialize display
    system.initializeDisplay();
    
    // Show text
    system.showText("Embedded System");
    
    // Run your system
    system.run();
    
    return 0;
}
```

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Display Class API

### Constructors
- `Display()` - Creates 400x400 window
- `Display(int width, int height)` - Creates custom sized window

### Methods
- `showWindow(const QString& text = "window")` - Shows window with specified text

## Example Output

The Display class creates a window with:
- **Size**: 400x400 pixels
- **Background**: White
- **Text**: "window" (default) or custom text
- **Font**: Arial, 16pt
- **Position**: Centered on screen

## Integration Notes

- The Display class inherits from `QMainWindow`
- Uses Qt's signal/slot mechanism (Q_OBJECT macro)
- Automatically handles window management and cleanup
- Thread-safe for integration with your existing System class 