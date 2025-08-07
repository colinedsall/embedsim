#include "display.hpp"
#include <QApplication>
#include <QScreen>
#include <QStyle>
#include <QStyleFactory>
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <iostream>

// CircleButton implementation
CircleButton::CircleButton(const QString& text, QWidget* parent) 
    : QWidget(parent), buttonText(text), isPressed(false), buttonSize(80)
{
    setFixedSize(buttonSize, buttonSize);
    setMouseTracking(true);
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

DisplayApp::DisplayApp() : windowWidth(400), windowHeight(400)
{
    setupUI();
}

DisplayApp::DisplayApp(int width, int height) : windowWidth(width), windowHeight(height)
{
    setupUI();
}

DisplayApp::~DisplayApp()
{
    // Disconnect signals to prevent callbacks after destruction
    if (circleButton) {
        disconnect(circleButton, nullptr, this, nullptr);
    }
    
    // Clear external handler to prevent dangling function calls
    externalClickHandler = nullptr;
    
    // Qt will handle widget cleanup automatically when parent is destroyed
    // No need to manually delete child widgets
}

void DisplayApp::setupUI()
{
    // Set window properties
    setWindowTitle("Embedded System Display");
    resize(windowWidth, windowHeight);
    setMinimumSize(300, 300); // Set minimum size for resizable window
    
    // Set white background
    setStyleSheet("QMainWindow { background-color: white; }");
    
    // Create central widget and layout
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);
    
    // Create text label
    textLabel = new QLabel(this);
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet("QLabel { background-color: white; color: black; font-size: 16px; }");
    textLabel->setMinimumHeight(50);
    
    // Set Arial font
    QFont arialFont("Arial", 16, QFont::Normal);
    textLabel->setFont(arialFont);
    
    // Create circle button
    circleButton = new CircleButton("Click Me!", this);
    
    // Create horizontal layout for button centering
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(circleButton);
    buttonLayout->addStretch();
    
    // Add widgets to main layout
    layout->addWidget(textLabel);
    layout->addLayout(buttonLayout);
    layout->addStretch(); // Add stretch to push content to top
    
    // Connect button signal
    if (circleButton) {
        connect(circleButton, &CircleButton::clicked, this, &DisplayApp::onCircleButtonClicked);
    }
    
    // Center the window on screen
    centerText();
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
    displayText = text;
    if (textLabel) {
        textLabel->setText(text);
    }
    
    // Show the window
    show();
    
    // Bring to front
    raise();
    activateWindow();
}

void DisplayApp::connectButtonClick(std::function<void()> handler)
{
    externalClickHandler = handler;
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