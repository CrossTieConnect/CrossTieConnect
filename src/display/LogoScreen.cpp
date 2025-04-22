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


#include "LogoScreen.h"
#include <M5StickCPlus2.h>
#include "../logo.h"

LogoScreen::LogoScreen() : 
    Screen(),
    activationTime(0),
    hasShownLogo(false) {
}

void LogoScreen::activate() {
    activationTime = millis();
    hasShownLogo = false;
    setNeedsRedraw();
}

void LogoScreen::deactivate() {
    // Nothing specific needed when deactivating
}

void LogoScreen::update() {
    if (redrawNeeded) {
        M5.Lcd.clear();
        
        if (!hasShownLogo) {
            // Set background color
            M5.Lcd.fillScreen(TFT_BLACK);
            
            // Draw image from embedded data
            M5.Lcd.startWrite();
            
            // Calculate center position
            int x = (M5.Lcd.width() - 135) / 2;  // Adjust based on actual image width
            int y = (M5.Lcd.height() - 135) / 2;  // Adjust based on actual image height
            
            // Draw the PNG directly from memory
            M5.Lcd.drawPng(logo_logo_png, logo_logo_png_len, x, y);
            
            M5.Lcd.endWrite();
            Serial.println("Logo displayed from memory");
            
            hasShownLogo = true;
        }
        
        redrawNeeded = false;
    }
}

void LogoScreen::handleButton(uint8_t button) {
    // No button handling in logo screen
} 