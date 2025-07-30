#ifndef SYSTEM_DISPLAY_HPP
#define SYSTEM_DISPLAY_HPP

#include "system.hpp"
#include "display.hpp"
#include <QApplication>
#include <memory>

class SystemWithDisplay : public System
{
public:
    SystemWithDisplay();
    ~SystemWithDisplay();
    
    // Display control methods
    void initializeDisplay();
    void showText(const QString& text);
    void closeDisplay();
    
private:
    std::unique_ptr<QApplication> qtApp;
    std::unique_ptr<Display> display;
    bool displayInitialized;
};

#endif 