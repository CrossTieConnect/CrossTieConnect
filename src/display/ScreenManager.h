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
#include <vector>
#include "Screen.h"

// Screen types
enum ScreenType {
    SCREEN_LOGO = 0,
    SCREEN_STATUS = 1,
    SCREEN_CONTROLLER = 2,
    SCREEN_CONNECTION = 3,
    // Future screens can be added here
    SCREEN_MENU = 4,
    SCREEN_SETTINGS = 5
};

class ScreenManager {
public:
    ScreenManager();
    ~ScreenManager();
    
    // Register a screen
    void registerScreen(ScreenType type, Screen* screen);
    
    // Switch to a screen
    void switchToScreen(ScreenType type);
    
    // Get current screen type
    ScreenType getCurrentScreenType() const;
    
    // Update current screen
    void update();
    
    // Handle button input
    void handleButton(uint8_t button);
    
private:
    std::vector<Screen*> screens;
    ScreenType currentScreenType;
    Screen* currentScreen;
}; 