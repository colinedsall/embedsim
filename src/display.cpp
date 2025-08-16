#include "display.hpp"
#include "graphics_objects.hpp"
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QStyleFactory>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QThread>
#include <iostream>
#include <algorithm>

// MiniDisplayWidget implementation
MiniDisplayWidget::MiniDisplayWidget(QWidget* parent)
    : QWidget(parent), graphicsManager(nullptr)
{
    setFixedSize(256, 256);
    setStyleSheet("QWidget { background-color: black; border: 2px solid #333; }");
}

void MiniDisplayWidget::setGraphicsManager(GraphicsManager* manager)
{
    graphicsManager = manager;
}

void MiniDisplayWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background
    painter.fillRect(rect(), Qt::black);
    
    // Draw graphics objects if manager exists
    if (graphicsManager) {
        graphicsManager->drawAll(painter);
    }
}

// CircleButton implementation
CircleButton::CircleButton(const QString& text, QWidget* parent) 
    : QWidget(parent), buttonText(text), isPressed(false), buttonSize(80)
{
    std::cout << "DEBUG: CircleButton constructor started" << std::endl;
    setFixedSize(buttonSize, buttonSize);
    setMouseTracking(true);
    std::cout << "DEBUG: CircleButton constructor completed" << std::endl;
}

CircleButton::~CircleButton()
{
}

void CircleButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Calculate center and radius
    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = buttonSize / 2 - 5;
    
    // Draw circle with different colors based on state
    QColor circleColor = isPressed ? QColor(100, 150, 255) : QColor(200, 200, 255);
    painter.setBrush(circleColor);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
    
    // Draw text
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 10, QFont::Bold));
    painter.drawText(rect(), Qt::AlignCenter, buttonText);
}

void CircleButton::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        isPressed = true;
        update(); // Trigger repaint
    }
}

void CircleButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        isPressed = false;
        update(); // Trigger repaint
        
        // Check if click is within the circle
        int centerX = width() / 2;
        int centerY = height() / 2;
        int radius = buttonSize / 2 - 5;
        
        QPoint clickPos = event->pos();
        int distanceSquared = (clickPos.x() - centerX) * (clickPos.x() - centerX) + 
                             (clickPos.y() - centerY) * (clickPos.y() - centerY);
        
        if (distanceSquared <= radius * radius) {
            emit clicked();
        }
    }
}

DisplayApp::DisplayApp() : windowWidth(800), windowHeight(500)
{
    std::cout << "DEBUG: DisplayApp constructor started" << std::endl;
    setupUI();
    std::cout << "DEBUG: DisplayApp constructor completed" << std::endl;
}

DisplayApp::DisplayApp(int width, int height) : windowWidth(width), windowHeight(height)
{
    std::cout << "DEBUG: DisplayApp constructor with size started" << std::endl;
    setupUI();
    std::cout << "DEBUG: DisplayApp constructor with size completed" << std::endl;
}

DisplayApp::~DisplayApp()
{
    std::cout << "DEBUG: DisplayApp destructor started" << std::endl;
    
    // Disconnect signals to prevent callbacks after destruction
    if (circleButton) {
        disconnect(circleButton, nullptr, this, nullptr);
        std::cout << "DEBUG: Circle button signals disconnected" << std::endl;
    }
    
    // Disconnect L button signals
    if (l1Button) {
        disconnect(l1Button, nullptr, this, nullptr);
        std::cout << "DEBUG: L1 button signals disconnected" << std::endl;
    }
    if (l2Button) {
        disconnect(l2Button, nullptr, this, nullptr);
        std::cout << "DEBUG: L2 button signals disconnected" << std::endl;
    }
    if (l3Button) {
        disconnect(l3Button, nullptr, this, nullptr);
        std::cout << "DEBUG: L3 button signals disconnected" << std::endl;
    }
    if (l4Button) {
        disconnect(l4Button, nullptr, this, nullptr);
        std::cout << "DEBUG: L4 button signals disconnected" << std::endl;
    }
    
    // Clear external handler to prevent dangling function calls
    externalClickHandler = nullptr;
    externalL1ClickHandler = nullptr;
    externalL2ClickHandler = nullptr;
    externalL3ClickHandler = nullptr;
    externalL4ClickHandler = nullptr;
    terminalCommandCallback = nullptr;
    std::cout << "DEBUG: External click handlers cleared" << std::endl;
    
    // Close the window properly
    close();
    std::cout << "DEBUG: Window closed" << std::endl;
    
    // Qt will handle widget cleanup automatically when parent is destroyed
    // No need to manually delete child widgets
    std::cout << "DEBUG: DisplayApp destructor completed" << std::endl;
}

void DisplayApp::setupUI()
{
    std::cout << "DEBUG: setupUI started" << std::endl;
    
    // Set window properties
    setWindowTitle("Embedded System Display");
    
    // Set application icon (will show in dock and system menus)
    QIcon appIcon(":/icons/app_icon.png");  // Resource path for the icon
    if (!appIcon.isNull()) {
        // Set icon at both application and window level for maximum compatibility
        QApplication::setWindowIcon(appIcon);  // This affects the dock
        setWindowIcon(appIcon);                // This affects the window title bar
        
        std::cout << "DEBUG: Application icon set successfully" << std::endl;
        std::cout << "DEBUG: Icon size: " << appIcon.actualSize(QSize(64, 64)).width() << "x" << appIcon.actualSize(QSize(64, 64)).height() << std::endl;
        std::cout << "DEBUG: Icon available sizes: ";
        for (const QSize& size : appIcon.availableSizes()) {
            std::cout << size.width() << "x" << size.height() << " ";
        }
        std::cout << std::endl;
    } else {
        std::cout << "DEBUG: Could not load application icon" << std::endl;
        std::cout << "DEBUG: Icon path attempted: :/icons/app_icon.png" << std::endl;
    }
    
    resize(windowWidth, windowHeight);
    setMinimumSize(800, 500); // Set minimum size for resizable window
    std::cout << "DEBUG: Window properties set" << std::endl;
    
    // Set white background
    setStyleSheet("QMainWindow { background-color: white; }");
    std::cout << "DEBUG: Style sheet set" << std::endl;
    
    // Create central widget and main layout
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    std::cout << "DEBUG: Central widget created" << std::endl;
    
    // Create main horizontal layout
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);
    std::cout << "DEBUG: Main layout created" << std::endl;
    
    // Setup mini display region (left side)
    setupMiniDisplay();
    std::cout << "DEBUG: Mini display setup completed" << std::endl;
    
    // Create right side control panel
    QVBoxLayout* rightPanel = new QVBoxLayout();
    rightPanel->setSpacing(20);
    std::cout << "DEBUG: Right panel layout created" << std::endl;
    
    // Create text label
    textLabel = new QLabel(this);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet("QLabel { background-color: white; color: black; font-size: 16px; }");
    textLabel->setMinimumHeight(50);
    textLabel->setText("Initializing..."); // Set initial text
    std::cout << "DEBUG: Text label created with initial text" << std::endl;
    
    // Set Arial font
    QFont arialFont("Arial", 16, QFont::Normal);
    textLabel->setFont(arialFont);
    std::cout << "DEBUG: Font set" << std::endl;
    
    // Create circle button
    circleButton = new CircleButton("Click Me!", this);
    std::cout << "DEBUG: Circle button created" << std::endl;
    
    // Create L1-L4 buttons with red coloring
    l1Button = new QPushButton("L1", this);
    l2Button = new QPushButton("L2", this);
    l3Button = new QPushButton("L3", this);
    l4Button = new QPushButton("L4", this);
    
    // Set red color for all L buttons
    QString redButtonStyle = "QPushButton { background-color: #ff4444; color: white; font-weight: bold; font-size: 14px; padding: 8px; border: 2px solid #cc0000; border-radius: 5px; } QPushButton:hover { background-color: #ff6666; } QPushButton:pressed { background-color: #cc0000; }";
    l1Button->setStyleSheet(redButtonStyle);
    l2Button->setStyleSheet(redButtonStyle);
    l3Button->setStyleSheet(redButtonStyle);
    l4Button->setStyleSheet(redButtonStyle);
    
    // Set fixed size for L buttons
    l1Button->setFixedSize(80, 40);
    l2Button->setFixedSize(80, 40);
    l3Button->setFixedSize(80, 40);
    l4Button->setFixedSize(80, 40);
    
    std::cout << "DEBUG: L1-L4 buttons created" << std::endl;
    
    // Create horizontal layout for button centering
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(l1Button);
    buttonLayout->addWidget(l2Button);
    buttonLayout->addWidget(l3Button);
    buttonLayout->addWidget(l4Button);
    buttonLayout->addWidget(circleButton);
    buttonLayout->addStretch();
    std::cout << "DEBUG: Button layout created" << std::endl;
    
    // Setup timer UI
    setupTimerUI();
    std::cout << "DEBUG: Timer UI setup completed" << std::endl;
    
    // Setup terminal UI
    setupTerminalUI();
    std::cout << "DEBUG: Terminal UI setup completed" << std::endl;
    
    // Add widgets to right panel layout
    rightPanel->addWidget(textLabel);
    rightPanel->addLayout(buttonLayout);
    rightPanel->addWidget(timerGroupBox);
    rightPanel->addWidget(terminalGroupBox);
    rightPanel->addStretch(); // Add stretch to push content to top
    std::cout << "DEBUG: Widgets added to right panel" << std::endl;
    
    // Add mini display and right panel to main layout
    mainLayout->addWidget(miniDisplayRegion);
    mainLayout->addLayout(rightPanel);
    std::cout << "DEBUG: Main layout populated" << std::endl;
    
    // Connect button signal
    if (circleButton) {
        connect(circleButton, &CircleButton::clicked, this, &DisplayApp::onCircleButtonClicked);
        std::cout << "DEBUG: Button signal connected" << std::endl;
    }
    
    // Connect L1-L4 button signals
    if (l1Button) {
        connect(l1Button, &QPushButton::clicked, this, &DisplayApp::onL1ButtonClicked);
        std::cout << "DEBUG: L1 button signal connected" << std::endl;
    }
    if (l2Button) {
        connect(l2Button, &QPushButton::clicked, this, &DisplayApp::onL2ButtonClicked);
        std::cout << "DEBUG: L2 button signal connected" << std::endl;
    }
    if (l3Button) {
        connect(l3Button, &QPushButton::clicked, this, &DisplayApp::onL3ButtonClicked);
        std::cout << "DEBUG: L3 button signal connected" << std::endl;
    }
    if (l4Button) {
        connect(l4Button, &QPushButton::clicked, this, &DisplayApp::onL4ButtonClicked);
        std::cout << "DEBUG: L4 button signal connected" << std::endl;
    }
    
    // Center the window on screen
    centerText();
    std::cout << "DEBUG: Window centered" << std::endl;
    
    // Make window visible by default
    setVisible(true);
    resize(windowWidth, windowHeight);
    std::cout << "DEBUG: Window set visible and sized to " << windowWidth << "x" << windowHeight << std::endl;
    
    std::cout << "DEBUG: setupUI completed successfully" << std::endl;
}

void DisplayApp::centerText()
{
    // Center the window on the screen
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - windowWidth) / 2;
        int y = (screenGeometry.height() - windowHeight) / 2;
        move(x, y);
    }
}

void DisplayApp::showWindow(const QString& text)
{
    std::cout << "DEBUG: showWindow started with text: " << text.toStdString() << std::endl;
    
    displayText = text;
    if (textLabel) {
        textLabel->setText(text);
        std::cout << "DEBUG: Text label updated with: " << text.toStdString() << std::endl;
        
        // Force update the label
        textLabel->update();
        textLabel->repaint();
        std::cout << "DEBUG: Text label forced update" << std::endl;
    } else {
        std::cerr << "ERROR: textLabel is null!" << std::endl;
    }
    
    // Just update the text, window is already visible
    std::cout << "DEBUG: Window already visible, just updating text" << std::endl;
    
    // Bring to front if needed
    if (isVisible()) {
        std::cout << "DEBUG: About to call raise()" << std::endl;
        raise();
        std::cout << "DEBUG: raise() completed" << std::endl;
        
        std::cout << "DEBUG: About to call activateWindow()" << std::endl;
        activateWindow();
        std::cout << "DEBUG: activateWindow() completed" << std::endl;
    }
    
    // Force a repaint of the entire window
    update();
    repaint();
    std::cout << "DEBUG: Window repaint forced" << std::endl;
    
    std::cout << "DEBUG: showWindow completed successfully" << std::endl;
}

void DisplayApp::connectButtonClick(std::function<void()> handler)
{
    externalClickHandler = handler;
}

void DisplayApp::connectL1ButtonClick(std::function<void()> handler)
{
    externalL1ClickHandler = handler;
}

void DisplayApp::connectL2ButtonClick(std::function<void()> handler)
{
    externalL2ClickHandler = handler;
}

void DisplayApp::connectL3ButtonClick(std::function<void()> handler)
{
    externalL3ClickHandler = handler;
}

void DisplayApp::connectL4ButtonClick(std::function<void()> handler)
{
    externalL4ClickHandler = handler;
}

void DisplayApp::onCircleButtonClicked()
{
    // Call external handler if connected
    if (externalClickHandler) {
        try {
            externalClickHandler();
        } catch (...) {
            // Prevent crashes from external handler exceptions
            std::cerr << "Error in external click handler\n";
        }
    }
    
    // Default behavior
    if (textLabel) {
        textLabel->setText("Button was clicked!");
    }
    
    QMessageBox::information(this, "Button Clicked", "Circle button was clicked!");
}

void DisplayApp::onL1ButtonClicked()
{
    // Call external handler if connected
    if (externalL1ClickHandler) {
        try {
            externalL1ClickHandler();
        } catch (...) {
            // Prevent crashes from external handler exceptions
            std::cerr << "Error in external L1 click handler\n";
        }
    }
    
    // Default behavior
    if (textLabel) {
        textLabel->setText("L1 button was clicked!");
    }
    
    QMessageBox::information(this, "Button Clicked", "L1 button was clicked!");
}

void DisplayApp::onL2ButtonClicked()
{
    // Call external handler if connected
    if (externalL2ClickHandler) {
        try {
            externalL2ClickHandler();
        } catch (...) {
            // Prevent crashes from external handler exceptions
            std::cerr << "Error in external L2 click handler\n";
        }
    }
    
    // Default behavior
    if (textLabel) {
        textLabel->setText("L2 button was clicked!");
    }
    
    QMessageBox::information(this, "Button Clicked", "L2 button was clicked!");
}

void DisplayApp::onL3ButtonClicked()
{
    // Call external handler if connected
    if (externalL3ClickHandler) {
        try {
            externalL3ClickHandler();
        } catch (...) {
            // Prevent crashes from external handler exceptions
            std::cerr << "Error in external L3 click handler\n";
        }
    }
    
    // Default behavior
    if (textLabel) {
        textLabel->setText("L3 button was clicked!");
    }
    
    QMessageBox::information(this, "Button Clicked", "L3 button was clicked!");
}

void DisplayApp::onL4ButtonClicked()
{
    // Call external handler if connected
    if (externalL4ClickHandler) {
        try {
            externalL4ClickHandler();
        } catch (...) {
            // Prevent crashes from external handler exceptions
            std::cerr << "Error in external L4 click handler\n";
        }
    }
    
    // Default behavior
    if (textLabel) {
        textLabel->setText("L4 button was clicked!");
    }
    
    QMessageBox::information(this, "Button Clicked", "L4 button was clicked!");
}

void DisplayApp::setupTimerUI()
{
    std::cout << "DEBUG: setupTimerUI started" << std::endl;
    
    // Create timer group box
    timerGroupBox = new QGroupBox("Timer Management", this);
    timerGroupBox->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    
    QGridLayout* timerLayout = new QGridLayout(timerGroupBox);
    timerLayout->setSpacing(10);
    
    // Timer name input
    QLabel* nameLabel = new QLabel("Timer Name:", this);
    timerNameEdit = new QLineEdit(this);
    timerNameEdit->setPlaceholderText("Enter timer name");
    timerNameEdit->setMaximumWidth(150);
    
    // Timer time input
    QLabel* timeLabel = new QLabel("Time (ms):", this);
    timerTimeEdit = new QLineEdit(this);
    timerTimeEdit->setPlaceholderText("Enter time in ms");
    timerTimeEdit->setMaximumWidth(100);
    
    // Clock cycles display
    QLabel* cyclesLabel = new QLabel("Clock Cycles:", this);
    clockCyclesLabel = new QLabel("0", this);
    clockCyclesLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; border: 1px solid #ccc; }");
    clockCyclesLabel->setMinimumWidth(100);
    
    // Timer control buttons
    addTimerButton = new QPushButton("Add Timer", this);
    startTimerButton = new QPushButton("Start Timer", this);
    stopTimerButton = new QPushButton("Stop Timer", this);
    removeTimerButton = new QPushButton("Remove Timer", this);
    
    // Timer status display
    QLabel* statusLabel = new QLabel("Timer Status:", this);
    timerStatusDisplay = new QTextEdit(this);
    timerStatusDisplay->setMaximumHeight(150);
    timerStatusDisplay->setReadOnly(true);
    timerStatusDisplay->setStyleSheet("QTextEdit { background-color: #f8f8f8; font-family: monospace; font-size: 12px; }");
    
    // Layout arrangement
    timerLayout->addWidget(nameLabel, 0, 0);
    timerLayout->addWidget(timerNameEdit, 0, 1);
    timerLayout->addWidget(timeLabel, 0, 2);
    timerLayout->addWidget(timerTimeEdit, 0, 3);
    timerLayout->addWidget(cyclesLabel, 0, 4);
    timerLayout->addWidget(clockCyclesLabel, 0, 5);
    
    timerLayout->addWidget(addTimerButton, 1, 0);
    timerLayout->addWidget(startTimerButton, 1, 1);
    timerLayout->addWidget(stopTimerButton, 1, 2);
    timerLayout->addWidget(removeTimerButton, 1, 3);
    
    timerLayout->addWidget(statusLabel, 2, 0, 1, 2);
    timerLayout->addWidget(timerStatusDisplay, 3, 0, 1, 6);
    
    // Connect button signals
    connect(addTimerButton, &QPushButton::clicked, this, &DisplayApp::onAddTimerClicked);
    connect(startTimerButton, &QPushButton::clicked, this, &DisplayApp::onStartTimerClicked);
    connect(stopTimerButton, &QPushButton::clicked, this, &DisplayApp::onStopTimerClicked);
    connect(removeTimerButton, &QPushButton::clicked, this, &DisplayApp::onRemoveTimerClicked);
    
    std::cout << "DEBUG: setupTimerUI completed" << std::endl;
}

void DisplayApp::setupTerminalUI()
{
    std::cout << "DEBUG: setupTerminalUI started" << std::endl;
    
    // Create terminal group box
    terminalGroupBox = new QGroupBox("Terminal/CLI", this);
    terminalGroupBox->setStyleSheet("QGroupBox { font-weight: bold; font-size: 14px; }");
    
    QVBoxLayout* terminalLayout = new QVBoxLayout(terminalGroupBox);
    terminalLayout->setSpacing(10);
    
    // Terminal output display
    terminalOutput = new QTextEdit(this);
    terminalOutput->setMaximumHeight(150);
    terminalOutput->setReadOnly(true);
    terminalOutput->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #ffffff; font-family: 'Courier New', monospace; font-size: 11px; border: 1px solid #333; }");
    
    // Add welcome message
    terminalOutput->append("Embedded System CLI Terminal");
    terminalOutput->append("Type 'help' for available commands");
    terminalOutput->append("----------------------------------------");
    
    // Terminal input area with horizontal layout
    QHBoxLayout* inputLayout = new QHBoxLayout();
    
    terminalInput = new QLineEdit(this);
    terminalInput->setPlaceholderText("Enter CLI command (e.g., help, status, start, stop)");
    terminalInput->setStyleSheet("QLineEdit { background-color: #2d2d2d; color: #ffffff; border: 1px solid #555; padding: 5px; font-family: 'Courier New', monospace; }");
    
    terminalSendButton = new QPushButton("Send", this);
    terminalSendButton->setStyleSheet("QPushButton { background-color: #0078d4; color: white; border: none; padding: 5px 15px; font-weight: bold; } QPushButton:hover { background-color: #106ebe; }");
    
    terminalClearButton = new QPushButton("Clear", this);
    terminalClearButton->setStyleSheet("QPushButton { background-color: #d83b01; color: white; border: none; padding: 5px 15px; font-weight: bold; } QPushButton:hover { background-color: #b02e01; }");
    
    inputLayout->addWidget(terminalInput);
    inputLayout->addWidget(terminalSendButton);
    inputLayout->addWidget(terminalClearButton);
    
    // Add widgets to terminal layout
    terminalLayout->addWidget(terminalOutput);
    terminalLayout->addLayout(inputLayout);
    
    // Connect terminal signals
    connect(terminalSendButton, &QPushButton::clicked, this, &DisplayApp::onTerminalSendClicked);
    connect(terminalClearButton, &QPushButton::clicked, this, &DisplayApp::onTerminalClearClicked);
    connect(terminalInput, &QLineEdit::returnPressed, this, &DisplayApp::onTerminalInputReturnPressed);
    
    std::cout << "DEBUG: setupTerminalUI completed" << std::endl;
}

void DisplayApp::setupMiniDisplay()
{
    std::cout << "DEBUG: setupMiniDisplay started" << std::endl;
    
    // Create custom mini display widget
    miniDisplayWidget = new MiniDisplayWidget(this);
    
    // Initialize graphics manager
    graphicsManager = std::make_unique<GraphicsManager>();
    
    // Connect graphics manager to the widget
    miniDisplayWidget->setGraphicsManager(graphicsManager.get());
    
    // Create a layout for the mini display
    QVBoxLayout* miniDisplayLayout = new QVBoxLayout(miniDisplayWidget);
    miniDisplayLayout->setContentsMargins(0, 0, 0, 0);
    miniDisplayLayout->setSpacing(0);
    
    // Add a label for graphics content (deprecated)
    // QLabel* graphicsLabel = new QLabel("Graphics Region", miniDisplayWidget);
    // graphicsLabel->setAlignment(Qt::AlignCenter);
    // graphicsLabel->setStyleSheet("QLabel { color: white; font-family: Arial; font-size: 14px; background-color: transparent; }");
    // miniDisplayLayout->addWidget(graphicsLabel);
    
    // Set the mini display region to the custom widget
    miniDisplayRegion = miniDisplayWidget;
    
    std::cout << "DEBUG: setupMiniDisplay completed" << std::endl;
}

void DisplayApp::updateClockCycles(int cycles)
{
    currentClockCycles = cycles;
    if (clockCyclesLabel) {
        clockCyclesLabel->setText(QString::number(cycles));
    }
    updateTimerDisplay();
}

void DisplayApp::updateTimerStatus(const std::vector<TimerDisplayItem>& timers)
{
    timerItems.clear();
    for (const auto& timer : timers) {
        timerItems.push_back(timer);
    }
    updateTimerDisplay();
}

void DisplayApp::addTimer(const std::string& name, int timeMs, std::shared_ptr<Timer> timer)
{
    TimerDisplayItem item;
    item.name = name;
    item.timeMs = timeMs;
    item.timer = timer;
    item.isRunning = false;
    item.currentCycles = 0;
    item.rolloverCount = 0;
    
    timerItems.push_back(item);
    updateTimerDisplay();
}

void DisplayApp::removeTimer(const std::string& name)
{
    timerItems.erase(
        std::remove_if(timerItems.begin(), timerItems.end(),
            [&name](const TimerDisplayItem& item) { return item.name == name; }),
        timerItems.end()
    );
    updateTimerDisplay();
}

void DisplayApp::updateTimerDisplay()
{
    if (!timerStatusDisplay) return;
    
    QString displayText;
    displayText += QString("Clock Cycles: %1\n\n").arg(currentClockCycles);
    
    if (timerItems.empty()) {
        displayText += "No timers configured\n";
    } else {
        displayText += "Active Timers:\n";
        // Use double tabs for consistent column separation
        displayText += "Name\tTime(ms)\tStatus\tCycles\tRollovers\n";
        displayText += "----\t--------\t------\t------\t---------\n";
        
        for (const auto& item : timerItems) {
            QString status = item.isRunning ? "RUNNING" : "STOPPED";
            displayText += QString("%1\t%2\t%3\t%4\t%5\n")
                .arg(QString::fromStdString(item.name))
                .arg(item.timeMs)
                .arg(status)
                .arg(item.currentCycles)
                .arg(item.rolloverCount);
        }
    }
    
    timerStatusDisplay->setText(displayText);
}

void DisplayApp::onAddTimerClicked()
{
    QString name = timerNameEdit->text().trimmed();
    QString timeText = timerTimeEdit->text().trimmed();
    
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a timer name.");
        return;
    }
    
    bool ok;
    int timeMs = timeText.toInt(&ok);
    if (!ok || timeMs <= 0) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid time in milliseconds.");
        return;
    }
    
    // Check if timer name already exists
    for (const auto& item : timerItems) {
        if (item.name == name.toStdString()) {
            QMessageBox::warning(this, "Input Error", "Timer name already exists.");
            return;
        }
    }
    
    // Call external callback if connected
    if (addTimerCallback) {
        addTimerCallback(name.toStdString(), timeMs);
    }
    
    // Clear input fields
    timerNameEdit->clear();
    timerTimeEdit->clear();
}

void DisplayApp::onStartTimerClicked()
{
    QString name = timerNameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a timer name to start.");
        return;
    }
    
    // Call external callback if connected
    if (startTimerCallback) {
        startTimerCallback(name.toStdString());
    }
}

void DisplayApp::onStopTimerClicked()
{
    QString name = timerNameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a timer name to stop.");
        return;
    }
    
    // Call external callback if connected
    if (stopTimerCallback) {
        stopTimerCallback(name.toStdString());
    }
}

void DisplayApp::onRemoveTimerClicked()
{
    QString name = timerNameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a timer name to remove.");
        return;
    }
    
    // Call external callback if connected
    if (removeTimerCallback) {
        removeTimerCallback(name.toStdString());
    }
    
    // Clear input field
    timerNameEdit->clear();
}

void DisplayApp::connectAddTimerCallback(std::function<void(const std::string&, int)> callback)
{
    addTimerCallback = callback;
}

void DisplayApp::connectStartTimerCallback(std::function<void(const std::string&)> callback)
{
    startTimerCallback = callback;
}

void DisplayApp::connectStopTimerCallback(std::function<void(const std::string&)> callback)
{
    stopTimerCallback = callback;
}

void DisplayApp::connectRemoveTimerCallback(std::function<void(const std::string&)> callback)
{
    removeTimerCallback = callback;
}

// Terminal/CLI interface methods
void DisplayApp::appendTerminalOutput(const QString& text)
{
    if (terminalOutput) {
        terminalOutput->append(text);
        // Auto-scroll to bottom
        QTextCursor cursor = terminalOutput->textCursor();
        cursor.movePosition(QTextCursor::End);
        terminalOutput->setTextCursor(cursor);
    }
}

void DisplayApp::clearTerminalOutput()
{
    if (terminalOutput) {
        terminalOutput->clear();
        // Re-add welcome message
        terminalOutput->append("Embedded System CLI Terminal");
        terminalOutput->append("Type 'help' for available commands");
        terminalOutput->append("----------------------------------------");
    }
}

void DisplayApp::connectTerminalCommand(std::function<void(const std::string&)> callback)
{
    terminalCommandCallback = callback;
}

// Terminal slot methods
void DisplayApp::onTerminalSendClicked()
{
    sendTerminalCommand();
}

void DisplayApp::onTerminalClearClicked()
{
    clearTerminalOutput();
}

void DisplayApp::onTerminalInputReturnPressed()
{
    sendTerminalCommand();
}

void DisplayApp::sendTerminalCommand()
{
    if (!terminalInput || terminalInput->text().trimmed().isEmpty()) {
        return;
    }
    
    QString command = terminalInput->text().trimmed();
    
    // Display command in terminal output
    appendTerminalOutput("CLI> " + command);
    
    // Call external callback if connected
    if (terminalCommandCallback) {
        try {
            terminalCommandCallback(command.toStdString());
        } catch (...) {
            appendTerminalOutput("Error: Failed to execute command");
        }
    }
    
    // Clear input field
    terminalInput->clear();
}

// Graphics drawing interface implementation
int DisplayApp::drawLine(int x1, int y1, int x2, int y2, const QString& colorHex)
{
    if (!graphicsManager) {
        return -1;
    }
    
    QColor color;
    if (colorHex.startsWith("#")) {
        color = QColor(colorHex);
    } else {
        color = QColor("#" + colorHex);
    }
    
    if (!color.isValid()) {
        return -1;
    }
    
    int id = graphicsManager->createLine(x1, y1, x2, y2, color);
    miniDisplayRegion->update(); // Trigger repaint
    return id;
}

int DisplayApp::drawRectangle(int x, int y, int width, int height, const QString& colorHex, bool solid)
{
    if (!graphicsManager) {
        return -1;
    }
    
    QColor color;
    if (colorHex.startsWith("#")) {
        color = QColor(colorHex);
    } else {
        color = QColor("#" + colorHex);
    }
    
    if (!color.isValid()) {
        return -1;
    }
    
    FillStyle fillStyle = solid ? FillStyle::Solid : FillStyle::Hollow;
    int id = graphicsManager->createRectangle(x, y, width, height, color, fillStyle);
    miniDisplayRegion->update(); // Trigger repaint
    return id;
}

int DisplayApp::drawCircle(int x, int y, int radius, const QString& colorHex, bool solid)
{
    if (!graphicsManager) {
        return -1;
    }
    
    QColor color;
    if (colorHex.startsWith("#")) {
        color = QColor(colorHex);
    } else {
        color = QColor("#" + colorHex);
    }
    
    if (!color.isValid()) {
        return -1;
    }
    
    FillStyle fillStyle = solid ? FillStyle::Solid : FillStyle::Hollow;
    int id = graphicsManager->createCircle(x, y, radius, color, fillStyle);
    miniDisplayRegion->update(); // Trigger repaint
    return id;
}

bool DisplayApp::removeGraphicsObject(int id)
{
    if (!graphicsManager) {
        return false;
    }
    
    bool result = graphicsManager->removeObject(id);
    if (result) {
        miniDisplayRegion->update(); // Trigger repaint
    }
    return result;
}

void DisplayApp::clearGraphics()
{
    if (graphicsManager) {
        graphicsManager->clearAll();
        miniDisplayRegion->update(); // Trigger repaint
    }
}

QString DisplayApp::getGraphicsInfo() const
{
    if (!graphicsManager) {
        return "Graphics manager not initialized";
    }
    return graphicsManager->getAllObjectsInfo();
}

size_t DisplayApp::getGraphicsMemoryUsage() const
{
    if (!graphicsManager) {
        return 0;
    }
    return graphicsManager->getMemoryUsage();
}

void DisplayApp::setObjectFillStyle(int id, bool solid)
{
    if (!graphicsManager) {
        return;
    }
    
    FillStyle fillStyle = solid ? FillStyle::Solid : FillStyle::Hollow;
    graphicsManager->setObjectFillStyle(id, fillStyle);
    miniDisplayRegion->update(); // Trigger repaint
}

void DisplayApp::paintMiniDisplay(QPainter& painter)
{
    if (graphicsManager) {
        graphicsManager->drawAll(painter);
    }
} 