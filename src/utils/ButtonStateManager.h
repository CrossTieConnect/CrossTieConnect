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


#pragma once

#include <Arduino.h>
#include "../Config.h"

class ButtonStateManager {
public:
    // Constructor
    ButtonStateManager(const char* buttonName, int stateCount = 2);
    
    // Change number of states (can be called at runtime)
    void setNumStates(int newNumStates);
    
    // Update button state based on current press
    void update(bool isPressed);
    
    // Get current value based on state - calculated on demand
    int getValue() const;
    
    // Get current state (for UI)
    int getState() const;
    
    // Reset state to default
    void reset();
    
private:
    const char* name;          // Button name for debugging
    int numStates;             // Number of states this button cycles through (>1 for toggle, ≤1 for momentary)
    int currentState;          // Current state (0 to numStates-1)
    bool wasPressed;           // Previous button state
    unsigned long lastPress;   // Time of last button press
    bool momentaryMode;        // True if button is momentary (not toggle)
}; 