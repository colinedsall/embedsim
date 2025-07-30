#ifndef IO_HPP
#define IO_HPP

#include <iostream>
#include <vector>
using namespace std;

enum class ButtonState {
    IDLE,
    PRESSED,
    RELEASED,
    DEBOUNCE
};

class Button {
public:
    string name;
    ButtonState state;
    bool enable;
    int debounceCount;
    static const int DEBOUNCE_THRESHOLD = 5;
    
    // Track the actual input state (what the button "sees")
    bool inputState;

    // Constructors, default to enabled
    Button() : state(ButtonState::IDLE), enable(true), debounceCount(0), inputState(false) {}
    Button(string n) : name(n), state(ButtonState::IDLE), enable(true), debounceCount(0), inputState(false) {}
};

class IO
{
    public:
        IO();
        IO(string name, bool enable);
        ~IO();
        int pressedCount = 0;

        void addButton(Button button);
        void pollButtons(bool inputState);
        void setButtonPressed(string buttonName, bool pressed);
        bool isButtonPressed(string buttonName) const;
        
        // New method to poll using actual button input states
        void pollButtonsWithStates();
        
        // Additional control methods
        void resetButton(string buttonName);
        bool getButtonInputState(string buttonName) const;
        
        // Getter for status reporting
        const vector<Button>& getButtons() const { return buttons; }

    private:
        string name;
        bool enable;
        vector<Button> buttons;
        
        // Helper functions
        Button* findButton(string buttonName);
        void updateButtonState(Button& button, bool inputState);

};

#endif