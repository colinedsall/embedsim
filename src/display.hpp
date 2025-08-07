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
#include <functional>

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

private slots:
    void onCircleButtonClicked();

private:
    void setupUI();
    void centerText();

    QLabel* textLabel = nullptr;
    QWidget* centralWidget = nullptr;
    QVBoxLayout* layout = nullptr;
    CircleButton* circleButton = nullptr;
    
    int windowWidth = 400;
    int windowHeight = 400;
    QString displayText;
    std::function<void()> externalClickHandler;
};

#endif 