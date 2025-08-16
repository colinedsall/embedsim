#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QFont>
#include <QString>
#include <QPushButton>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QGroupBox>
#include <QGridLayout>
#include <QPainter>
#include <QPainterPath>
#include <functional>
#include <vector>
#include <memory>
#include <string>
#include "graphics_objects.hpp"

// Custom mini display widget that handles paint events
class MiniDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    MiniDisplayWidget(QWidget* parent = nullptr);
    void setGraphicsManager(GraphicsManager* manager);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    GraphicsManager* graphicsManager;
};

// Forward declaration
class Timer;

// Custom circle button widget
class CircleButton : public QWidget
{
    Q_OBJECT

public:
    CircleButton(const QString& text, QWidget* parent = nullptr);
    ~CircleButton();

// Required for the signal-slot mechanism (interaction between GUI and embedded system)
signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;


private:
    QString buttonText;
    bool isPressed;
    int buttonSize;
};

// Timer display item for the UI
struct TimerDisplayItem {
    std::string name;
    int timeMs;
    std::shared_ptr<Timer> timer;
    bool isRunning;
    int currentCycles;
    int rolloverCount;
};

class DisplayApp : public QMainWindow
{
    Q_OBJECT

public:
    DisplayApp();
    DisplayApp(int width, int height);
    ~DisplayApp();

    // Main display function
    void showWindow(const QString& text = "window");
    
    // Connect external signal handler
    void connectButtonClick(std::function<void()> handler);
    
    // Connect L1-L4 button signal handlers
    void connectL1ButtonClick(std::function<void()> handler);
    void connectL2ButtonClick(std::function<void()> handler);
    void connectL3ButtonClick(std::function<void()> handler);
    void connectL4ButtonClick(std::function<void()> handler);
    
    // Timer management functions
    void updateClockCycles(int cycles);
    void updateTimerStatus(const std::vector<TimerDisplayItem>& timers);
    void addTimer(const std::string& name, int timeMs, std::shared_ptr<Timer> timer);
    void removeTimer(const std::string& name);
    
    // Connect timer management callbacks
    void connectAddTimerCallback(std::function<void(const std::string&, int)> callback);
    void connectStartTimerCallback(std::function<void(const std::string&)> callback);
    void connectStopTimerCallback(std::function<void(const std::string&)> callback);
    void connectRemoveTimerCallback(std::function<void(const std::string&)> callback);
    
    // Terminal/CLI interface
    void appendTerminalOutput(const QString& text);
    void clearTerminalOutput();
    void connectTerminalCommand(std::function<void(const std::string&)> callback);
    
    // Graphics drawing interface for CLI
    int drawLine(int x1, int y1, int x2, int y2, const QString& colorHex);
    int drawRectangle(int x, int y, int width, int height, const QString& colorHex, bool solid = true);
    int drawCircle(int x, int y, int radius, const QString& colorHex, bool solid = true);
    bool removeGraphicsObject(int id);
    void clearGraphics();
    QString getGraphicsInfo() const;
    size_t getGraphicsMemoryUsage() const;
    void setObjectFillStyle(int id, bool solid);
    
    // Mini display interface
    QWidget* getMiniDisplayRegion() const { return miniDisplayRegion; }

private slots:
    void onCircleButtonClicked();
    void onL1ButtonClicked();
    void onL2ButtonClicked();
    void onL3ButtonClicked();
    void onL4ButtonClicked();
    void onAddTimerClicked();
    void onStartTimerClicked();
    void onStopTimerClicked();
    void onRemoveTimerClicked();
    void onTerminalSendClicked();
    void onTerminalClearClicked();
    void onTerminalInputReturnPressed();

private:
    void setupUI();
    void setupTimerUI();
    void setupTerminalUI();
    void setupMiniDisplay();
    void centerText();
    void updateTimerDisplay();
    void sendTerminalCommand();
    
    // Custom paint event for mini display
    void paintMiniDisplay(QPainter& painter);

    QLabel* textLabel = nullptr;
    QWidget* centralWidget = nullptr;
    QVBoxLayout* layout = nullptr;
    CircleButton* circleButton = nullptr;
    
    // L1-L4 buttons
    QPushButton* l1Button = nullptr;
    QPushButton* l2Button = nullptr;
    QPushButton* l3Button = nullptr;
    QPushButton* l4Button = nullptr;
    
    // Timer UI elements
    QGroupBox* timerGroupBox = nullptr;
    QLineEdit* timerNameEdit = nullptr;
    QLineEdit* timerTimeEdit = nullptr;
    QPushButton* addTimerButton = nullptr;
    QPushButton* startTimerButton = nullptr;
    QPushButton* stopTimerButton = nullptr;
    QPushButton* removeTimerButton = nullptr;
    QTextEdit* timerStatusDisplay = nullptr;
    QLabel* clockCyclesLabel = nullptr;
    
    // Terminal/CLI UI elements
    QGroupBox* terminalGroupBox = nullptr;
    QTextEdit* terminalOutput = nullptr;
    QLineEdit* terminalInput = nullptr;
    QPushButton* terminalSendButton = nullptr;
    QPushButton* terminalClearButton = nullptr;
    
    // Mini display region
    QWidget* miniDisplayRegion = nullptr;
    
    // Graphics manager for mini display
    std::unique_ptr<GraphicsManager> graphicsManager;
    
    // Custom mini display widget for drawing
    MiniDisplayWidget* miniDisplayWidget = nullptr;
    
    int windowWidth = 800;
    int windowHeight = 500; // Window dimensions for the display
    QString displayText;
    std::function<void()> externalClickHandler;
    
    // External L1-L4 button handlers
    std::function<void()> externalL1ClickHandler;
    std::function<void()> externalL2ClickHandler;
    std::function<void()> externalL3ClickHandler;
    std::function<void()> externalL4ClickHandler;
    
    // External timer management callbacks
    std::function<void(const std::string&, int)> addTimerCallback;
    std::function<void(const std::string&)> startTimerCallback;
    std::function<void(const std::string&)> stopTimerCallback;
    std::function<void(const std::string&)> removeTimerCallback;
    
    // External terminal command callback
    std::function<void(const std::string&)> terminalCommandCallback;
    
    // Timer management
    std::vector<TimerDisplayItem> timerItems;
    int currentClockCycles = 0;
};

#endif 