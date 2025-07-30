#include "system_display.hpp"
#include <iostream>

SystemWithDisplay::SystemWithDisplay() : displayInitialized(false)
{
    // System constructor will be called automatically
}

SystemWithDisplay::~SystemWithDisplay()
{
    closeDisplay();
}

void SystemWithDisplay::initializeDisplay()
{
    if (!displayInitialized) {
        // Create Qt application if it doesn't exist
        if (!qtApp) {
            int argc = 1;
            char* argv[] = {(char*)"embedsim"};
            qtApp = std::make_unique<QApplication>(argc, argv);
        }
        
        // Create display
        display = std::make_unique<Display>(400, 400);
        displayInitialized = true;
        
        std::cout << "Display initialized.\n";
    }
}

void SystemWithDisplay::showText(const QString& text)
{
    if (!displayInitialized) {
        initializeDisplay();
    }
    
    if (display) {
        display->showWindow(text);
        std::cout << "Displaying text: " << text.toStdString() << "\n";
    }
}

void SystemWithDisplay::closeDisplay()
{
    if (display) {
        display->close();
        display.reset();
    }
    displayInitialized = false;
} 