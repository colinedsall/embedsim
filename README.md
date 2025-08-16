# Embedded System Simulator

A C++ simulation framework for embedded systems with clock, timer, I/O, interrupt-like functionality, and a modern Qt-based display interface.

## System Simulation Features

- **Clock System**: Configurable clock with nanosecond simulation time
- **Timer Module**: Count-up timers with rollover detection and GUI management
- **I/O System**: Button simulation with FSM-based debouncing
- **Interrupt-like CLI**: Real-time user input handling via separate thread
- **Direct Clock Control**: Stop, start, pause, and resume clock via CLI
- **Thread-Safe**: Atomic operations for thread communication
- **Modern Display Interface**: Qt-based GUI with interactive controls and terminal
- **Interactive Buttons**: L1-L4 buttons with custom styling and signal handling
- **Integrated Terminal**: Built-in CLI terminal within the display window
- **Timer Management**: Add, start, stop, and remove timers through the GUI

## Usage

### Running the System
```bash
mkdir build && cd build
cmake ..
make
./embedsim
```

### Display Interface

The system now features a modern Qt-based display window (800x500 pixels) with:

- **Left Panel**: Mini display region (256x256) for future graphics
- **Right Panel**: Interactive controls and terminal
  - **Text Display**: Shows system status and messages
  - **Control Buttons**: L1, L2, L3, L4 (red) and Circle button
  - **Timer Management**: Add, start, stop, and remove timers
  - **Integrated Terminal**: Built-in CLI for real-time commands
  - **Clock Cycles Display**: Real-time clock cycle counter

### CLI Commands (Real-time Input)

You can use these commands in **both** the raw terminal and the integrated display terminal:

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
- `close` - Close the display window (keeps program running)
- `exit` - Close the display window and exit the program
- `help` - Show available commands

#### Timer Management (GUI + CLI)
- Add timers through the GUI interface
- Start/stop timers via GUI buttons or CLI commands
- Real-time timer status display
- Timer rollover detection and cycle counting

### Example Session

#### Display Terminal
```
Embedded System CLI Terminal
Type 'help' for available commands
----------------------------------------
CLI> status
=== System Status ===
Clock running: YES
Clock paused: NO
Global flag: OFF
Clock cycles: 0
Button aButton: Input=LOW, State=IDLE
===================

CLI> press aButton
Simulated button press for: aButton

CLI> status
=== System Status ===
Clock running: YES
Clock paused: NO
Global flag: OFF
Clock cycles: 15
Button aButton: Input=HIGH, State=PRESSED
===================

CLI> exit
Exiting program...
Display window closed.
Exit requested. Terminating program...
```

#### Raw Terminal (still available)
```
CLI> status
Clock running: YES
Clock paused: NO
Global flag: OFF
Clock cycles: 0
Button aButton: Input=LOW, State=IDLE

CLI> stop
Interrupt: Stopping clock
System stopped.
```

## Architecture

### Modern GUI Architecture
- **Qt Integration**: Cross-platform GUI framework
- **Event-Driven**: Button clicks and terminal input handled through Qt signals
- **Thread-Safe Communication**: GUI and system communicate through callback functions
- **Responsive Interface**: Real-time updates without blocking system simulation

### Clock-Synchronized Operations
- Main polling loop runs synchronized with clock cycles
- I/O polling and timer operations occur on positive clock edges
- Atomic flags allow real-time communication between threads
- GUI updates synchronized with system state

### Interrupt-like System
- CLI runs in separate thread for real-time input
- **Dual CLI Support**: Both raw terminal and integrated display terminal
- Atomic variables provide thread-safe communication
- Interrupt handlers can be registered for custom actions
- Direct clock control through interrupt triggers

### Display Integration
- **Window Management**: 800x500 pixel main window with organized layout
- **Button Integration**: L1-L4 buttons with custom styling and signal handling
- **Terminal Integration**: Built-in CLI terminal within the display
- **Timer GUI**: Visual timer management interface
- **Real-time Updates**: System status displayed in real-time

## Key Components

- **System**: Main orchestrator with interrupt handling, clock control, and display management
- **DisplayApp**: Qt-based display interface with integrated terminal and controls
- **Clock**: Threaded clock with configurable frequency and direct control
- **IO**: Button simulation with FSM debouncing
- **Timer**: Count-up timers with rollover detection and GUI management

## Display Module Features

### Window Layout
- **Size**: 800x500 pixels (configurable)
- **Left Panel**: Mini display region (256x256) for future graphics
- **Right Panel**: Interactive controls and terminal
- **Responsive Design**: Adapts to different screen sizes

### Interactive Elements
- **L1-L4 Buttons**: Red-styled buttons with custom signal handling
- **Circle Button**: Interactive button with click detection
- **Timer Controls**: Add, start, stop, and remove timers
- **Terminal Interface**: Built-in CLI with command history

### Terminal Features
- **Command Input**: Type commands directly in the display
- **Output Display**: Real-time command output and system status
- **Command History**: Previous commands visible in terminal
- **Error Handling**: Graceful error messages for invalid commands

### Timer Management Interface
- **Add Timer**: Specify name and duration
- **Start/Stop**: Control individual timers
- **Status Display**: Real-time timer information
- **Remove Timer**: Clean up unused timers

### Drawing on the Display
**Examples**
``` bash
# Draw a red line from (10,10) to (100,100)
line 10 10 100 100 FF0000

# Draw a blue solid rectangle at (50,50) with size 80x60
rect 50 50 80 60 0000FF

# Draw a blue hollow rectangle at (50,50) with size 80x60
rect 50 50 80 60 0000FF hollow

# Draw a green solid circle at (150,150) with radius 40
circle 150 150 40 00FF00

# Draw a green hollow circle at (150,150) with radius 40
circle 150 150 40 00FF00 hollow

# Change object with ID 2 to hollow fill style
fillstyle 2 hollow

# Remove object with ID 2
remove 2

# Show all graphics objects
graphics

# Show memory usage
memory

# Clear all graphics
clear
```

**Fill Style Options:**
- **Solid** (default): Objects are filled with the specified color
- **Hollow**: Objects have only borders in the specified color, no fill
- Use `solid` or `hollow` as the last parameter for rectangles and circles
- Use `fillstyle <id> <solid|hollow>` to change existing objects

## Requirements

- **Qt6** (Qt5 fallback supported)
- **CMake 3.13** or higher
- **C++14** or higher
- **Cross-platform**: macOS, Windows, Linux

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

## Building

```bash
mkdir build && cd build
cmake ..
make
```

## Application Icon

The application includes a custom icon that appears in:
- **Dock** (macOS)
- **Taskbar** (Windows)
- **Window title bar**
- **System menus**

Icon file: `icons/app_icon.png` (automatically included via Qt resources)

## Integration Notes

- **Dual CLI Support**: Use either the raw terminal or integrated display terminal
- **Thread Safety**: GUI and system simulation run in separate threads
- **Real-time Updates**: System status updates in real-time through the GUI
- **Clean Exit**: Both `close` and `exit` commands work properly
- **Modern Interface**: Professional-looking GUI with intuitive controls

## Future Enhancements

- **Graphics Rendering**: Mini display region for custom graphics
- **Custom Themes**: Configurable color schemes and styling
- **Plugin System**: Extensible architecture for additional features
- **Network Support**: Remote monitoring and control capabilities 