/*
 * Copyright (c) 2024 CrossTieConnect
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include "ButtonStateManager.h"

ButtonStateManager::ButtonStateManager(const char* buttonName, int stateCount) {
    name = buttonName;
    currentState = 0;
    wasPressed = false;
    lastPress = 0;
    
    // Set number of states and determine mode
    // stateCount ≤ 1 means momentary mode, otherwise it's a toggle with stateCount positions
    setNumStates(stateCount);
}

void ButtonStateManager::setNumStates(int newNumStates) {
    // State count ≤ 1 means momentary mode
    momentaryMode = (newNumStates <= 1);
    
    if (momentaryMode) {
        numStates = 2; // Momentary buttons still have 2 internal states (pressed/not pressed)
    } else {
        numStates = max(2, newNumStates); // Toggles need at least 2 states
    }
    
    currentState = 0;  // Reset to first state
    
    Serial.printf("Button %s config changed: %s with %d states\n", 
                name, 
                momentaryMode ? "momentary" : "toggle", 
                momentaryMode ? 2 : numStates);
}

void ButtonStateManager::update(bool isPressed) {
    unsigned long currentTime = millis();
    
    if (momentaryMode) {
        // Momentary mode: state follows the button (pressed or not)
        currentState = isPressed ? 1 : 0;
    } else {
        // Toggle mode: state changes on button press
        if (isPressed && !wasPressed) {
            // Button just pressed
            // Check for double press
            if (currentTime - lastPress < DOUBLE_PRESS_TIME) {
                // Double press detected - reset to first state
                currentState = 0;
                Serial.printf("Button %s: double-press reset\n", name);
            } else {
                // Single press - advance to next state
                currentState = (currentState + 1) % numStates;
                Serial.printf("Button %s: state %d/%d\n", name, currentState + 1, numStates);
            }
            
            lastPress = currentTime;
        }
    }
    
    wasPressed = isPressed;
}

int ButtonStateManager::getValue() const {
    if (momentaryMode) {
        // Momentary: simply MIN or MAX based on pressed state
        return currentState ? CHANNEL_VALUE_MAX : CHANNEL_VALUE_MIN;
    } else {
        // Multi-position toggle: distribute values evenly between MIN and MAX
        if (numStates == 2) {
            // 2-position toggle: MIN or MAX
            return (currentState == 0) ? CHANNEL_VALUE_MIN : CHANNEL_VALUE_MAX;
        } else if (numStates == 3 && currentState == 1) {
            // Special case for 3-position toggle: use the defined MID value
            return CHANNEL_VALUE_MID;
        } else {
            // For other multi-state toggles, distribute values evenly
            return CHANNEL_VALUE_MIN + 
                ((CHANNEL_VALUE_MAX - CHANNEL_VALUE_MIN) * currentState / (numStates - 1));
        }
    }
}

int ButtonStateManager::getState() const {
    return currentState;
}

void ButtonStateManager::reset() {
    currentState = 0;
    wasPressed = false;
} 