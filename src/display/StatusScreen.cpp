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


#include "StatusScreen.h"

StatusScreen::StatusScreen(Controller* controller) : 
    Screen(),
    controller(controller),
    statusMessage("WAITING") {
}

void StatusScreen::activate() {
    setNeedsRedraw();
}

void StatusScreen::deactivate() {
    // Nothing specific needed when deactivating
}

void StatusScreen::update() {
    String currentStatus = controller->getStatusMessage();
    
    // Only redraw if status changed or redraw is needed
    if (currentStatus != statusMessage || redrawNeeded) {
        statusMessage = currentStatus;
        
        // Clear the screen
        M5.Lcd.clear();
        
        // Title
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(2);
        M5.Lcd.setCursor(0, 30);
        M5.Lcd.setTextDatum(MC_DATUM);
        M5.Lcd.drawString("Status", M5.Lcd.width() / 2, 30);
        
        // Status
        M5.Lcd.drawString(statusMessage, M5.Lcd.width() / 2, 80);
        
        // Connection indicator
        if (controller->isConnected()) {
            M5.Lcd.fillCircle(M5.Lcd.width() / 2, M5.Lcd.height() - 15, 5, TFT_GREEN);
        } else {
            M5.Lcd.drawCircle(M5.Lcd.width() / 2, M5.Lcd.height() - 15, 5, TFT_RED);
        }
        
        redrawNeeded = false;
    }
}

void StatusScreen::handleButton(uint8_t button) {
    // No button handling in status screen
} 