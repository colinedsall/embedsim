#include "io.hpp"

IO::IO()
{
    cout << "Warning: Default constructor called. IO module will have no features.";
}

IO::IO(string name, bool enable) : name(name), enable(enable) {}

IO::~IO() {}

void IO::addButton(Button button)
{
    buttons.push_back(button);
}

void IO::pollButtons(bool inputState)
{
    for (Button& button : buttons) {
        if (button.enable) {
            updateButtonState(button, inputState);
        }
    }
}

void IO::pollButtonsWithStates()
{
    for (Button& button : buttons) {
        if (button.enable) {
            updateButtonState(button, button.inputState);
        }
    }
}

void IO::setButtonPressed(string buttonName, bool pressed)
{
    Button* button = findButton(buttonName);
    if (button && button->enable) {
        // Set the input state that the button "sees"
        button->inputState = pressed;
        // Optionally trigger immediate state update
        updateButtonState(*button, pressed);
    }
}

bool IO::isButtonPressed(string buttonName) const
{
    for (const Button& button : buttons) {
        if (button.name == buttonName) {
            return button.state == ButtonState::PRESSED;
        }
    }
    return false;
}

void IO::resetButton(string buttonName)
{
    Button* button = findButton(buttonName);
    if (button) {
        button->state = ButtonState::IDLE;
        button->inputState = false;
        button->debounceCount = 0;
    }
}

bool IO::getButtonInputState(string buttonName) const
{
    for (const Button& button : buttons) {
        if (button.name == buttonName) {
            return button.inputState;
        }
    }

    // Default to false if button not found
    return false;
}

Button* IO::findButton(string buttonName)
{
    for (Button& button : buttons) {
        if (button.name == buttonName) {
            return &button;
        }
    }
    return nullptr;
}

void IO::updateButtonState(Button& button, bool inputState)
{
    switch (button.state) {
        case ButtonState::IDLE:
            if (inputState) {
                button.state = ButtonState::DEBOUNCE;
                button.debounceCount = 1;
            }

            // cout << "Button " << button.name << " is in IDLE state. \n";
            break;
            
        case ButtonState::DEBOUNCE:
            if (inputState) {
                button.debounceCount++;
                // Directly compare with static threshold in Button class 
                if (button.debounceCount >= Button::DEBOUNCE_THRESHOLD) {
                    button.state = ButtonState::PRESSED;
                    button.debounceCount = 0;
                }
            } else {
                // Input went low during debounce, go back to idle
                button.state = ButtonState::IDLE;
                button.debounceCount = 0;
            }

            // cout << "Button " << button.name << " is in DEBOUNCE state. \n";
            // cout << "Debounce count: " << button.debounceCount << endl;
            // cout << "Leaving debounce state (y/n)?" << inputState << endl;
            break;
            
        case ButtonState::PRESSED:
            if (!inputState) {
                button.state = ButtonState::RELEASED;
            }
            pressedCount++;
            // cout << "Button " << button.name << " is in PRESSED state. \n";
            break;
            
        case ButtonState::RELEASED:
            // Stay in released state until explicitly reset
            // This allows the system to detect the release event
            // cout << "Button " << button.name << " is in RELEASED state. \n";
            break;
    }
}
