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


#include "ScreenManager.h"

ScreenManager::ScreenManager() : 
    currentScreenType(SCREEN_STATUS),
    currentScreen(nullptr) {
}

ScreenManager::~ScreenManager() {
    // Clean up screens
    for (auto screen : screens) {
        delete screen;
    }
    screens.clear();
}

void ScreenManager::registerScreen(ScreenType type, Screen* screen) {
    // Ensure the vector has enough capacity
    if (screens.size() <= type) {
        screens.resize(type + 1, nullptr);
    }
    
    // Store the screen
    screens[type] = screen;
    
    // If this is the first screen, make it active
    if (currentScreen == nullptr) {
        currentScreen = screen;
        currentScreenType = type;
        currentScreen->activate();
    }
}

void ScreenManager::switchToScreen(ScreenType type) {
    // Check if screen exists
    if (type >= screens.size() || screens[type] == nullptr) {
        Serial.println("Screen not available");
        return;
    }
    
    // Skip if already on this screen
    if (type == currentScreenType) {
        return;
    }
    
    // Deactivate current screen
    if (currentScreen != nullptr) {
        currentScreen->deactivate();
    }
    
    // Switch to new screen
    currentScreenType = type;
    currentScreen = screens[type];
    
    // Activate new screen
    if (currentScreen != nullptr) {
        currentScreen->activate();
    }
}

ScreenType ScreenManager::getCurrentScreenType() const {
    return currentScreenType;
}

void ScreenManager::update() {
    if (currentScreen != nullptr) {
        currentScreen->update();
    }
}

void ScreenManager::handleButton(uint8_t button) {
    if (currentScreen != nullptr) {
        currentScreen->handleButton(button);
    }
} 