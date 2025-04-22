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


#include "ControllerScreen.h"
#include "../Config.h"
#include "../channels/ChannelManager.h"

ControllerScreen::ControllerScreen(Controller* controller, ChannelManager* channelManager) : 
    Screen(),
    controller(controller),
    channelManager(channelManager),
    prevLX(0), prevLY(0), prevRX(0), prevRY(0),
    prevL2(0), prevR2(0) {
}

void ControllerScreen::activate() {
    // Mark as needing redraw
    setNeedsRedraw();
    
    // Force drawing of all elements by marking their previous values as different
    // This ensures they'll be drawn right away, even if there's no controller input yet
    prevLX = 1000; // Use an invalid value to force drawing
    prevLY = 1000;
    prevRX = 1000;
    prevRY = 1000;
    prevL2 = -1;   // Use an invalid value to force drawing
    prevR2 = -1;
}

void ControllerScreen::deactivate() {
    // Nothing specific needed when deactivating
}

void ControllerScreen::update() {
    // Get values from ChannelManager instead of controller
    // Map CRSF values (172-1811) to controller range (-128 to 127 for sticks, 0-255 for triggers)
    int lx = map(channelManager->getChannel(0), CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, -128, 127);
    int ly = map(channelManager->getChannel(2), CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, -128, 127);
    int rx = map(channelManager->getChannel(3), CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, -128, 127);
    int ry = map(channelManager->getChannel(1), CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, -128, 127);

    int l2 = map(channelManager->getChannel(6), CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
    int r2 = map(channelManager->getChannel(7), CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
    
    int centerX = M5.Lcd.width() / 2;
    int centerY = M5.Lcd.height() / 2;
    
    // Check if we need to do a full redraw
    if (redrawNeeded) {
        // Clear the screen
        M5.Lcd.clear();
        
        // Draw title
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextDatum(TC_DATUM);
        M5.Lcd.drawString("PS5 Controller", centerX, 5);
        
        // Force draw all UI elements
        drawStick(centerX - 33, centerY - 60, lx, ly, 1000, 1000, TFT_CYAN);
        drawStick(centerX + 33, centerY - 60, rx, ry, 1000, 1000, TFT_ORANGE);
        drawShoulderButtons(centerX, centerY - 20);
        drawTrigger(centerX, centerY, l2, r2, -1, -1, TFT_YELLOW);
        drawDPad(centerX - 33, centerY + 50);
        drawButtons(centerX + 33, centerY + 50);
        
        // Connection indicator
        M5.Lcd.fillCircle(M5.Lcd.width() - 10, 10, 3, controller->isConnected() ? TFT_GREEN : TFT_RED);
        
        redrawNeeded = false;
    } else {
        // Draw the stick visuals
        drawStick(centerX - 33, centerY - 60, lx, ly, prevLX, prevLY, TFT_CYAN);
        drawStick(centerX + 33, centerY - 60, rx, ry, prevRX, prevRY, TFT_ORANGE);
        
        // Draw L1 and R1
        drawShoulderButtons(centerX, centerY - 20);
        
        // Draw trigger bars
        drawTrigger(centerX, centerY, l2, r2, prevL2, prevR2, TFT_YELLOW);
        
        // Draw button indicators and D-pad which need to update each frame
        drawDPad(centerX - 33, centerY + 50);
        drawButtons(centerX + 33, centerY + 50);
        
        // Connection indicator - redraw if connection state changes
        static bool lastConnectionState = false;
        bool currentConnectionState = controller->isConnected();
        if (currentConnectionState != lastConnectionState) {
            M5.Lcd.fillCircle(M5.Lcd.width() - 10, 10, 3, currentConnectionState ? TFT_GREEN : TFT_RED);
            lastConnectionState = currentConnectionState;
        }
    }
    
    // Update previous values
    prevLX = lx;
    prevLY = ly;
    prevRX = rx;
    prevRY = ry;
    prevL2 = l2;
    prevR2 = r2;
}

void ControllerScreen::handleButton(uint8_t button) {
    // No button handling in controller screen yet
}

void ControllerScreen::drawStick(int x, int y, int valueX, int valueY, int prevX, int prevY, uint16_t color) {
    // Define smaller stick radius for portrait mode
    const int stickRadius = 15;
    
    // Map PS5 stick values (-128 to 127) to our display area
    int posX = map(valueX, -128, 127, -stickRadius, stickRadius);
    int posY = map(valueY, -128, 127, stickRadius, -stickRadius); // Invert Y axis
    
    int prevPosX = map(prevX, -128, 127, -stickRadius, stickRadius);
    int prevPosY = map(prevY, -128, 127, stickRadius, -stickRadius);
    
    // Only redraw if position changed or forced redraw
    if (posX != prevPosX || posY != prevPosY || prevX == 1000 || prevY == 1000 || redrawNeeded) {
        // Clear previous position if needed and valid
        if (prevX != 0 && prevY != 0 && prevX != 1000 && prevY != 1000) {
            M5.Lcd.fillCircle(x + prevPosX, y + prevPosY, 5, TFT_BLACK);
        }
        
        // Draw outer circle
        M5.Lcd.drawCircle(x, y, stickRadius, TFT_DARKGREY);
        
        // Draw position marker
        M5.Lcd.fillCircle(x + posX, y + posY, 5, color);
        
        // Draw crosshair
        M5.Lcd.drawFastHLine(x - stickRadius, y, stickRadius * 2, TFT_DARKGREY);
        M5.Lcd.drawFastVLine(x, y - stickRadius, stickRadius * 2, TFT_DARKGREY);
    }
}

void ControllerScreen::drawTrigger(int x, int y, int l2Value, int r2Value, int prevL2, int prevR2, uint16_t color) {
    // Sized for portrait display
    const int triggerWidth = 30; // Increased width
    const int triggerHeight = 8;
    const int TRIGGER_SPACING = 5;
    
    // Calculate positions
    int leftX = x - triggerWidth - TRIGGER_SPACING;
    int rightX = x + TRIGGER_SPACING;
    
    // Only redraw if values changed or forced redraw
    if (l2Value != prevL2 || r2Value != prevR2 || prevL2 < 0 || prevR2 < 0 || redrawNeeded) {
        int l2Width = map(l2Value, 0, 255, 0, triggerWidth);
        int r2Width = map(r2Value, 0, 255, 0, triggerWidth);
        
        // Clear previous L2 if needed
        if (prevL2 >= 0) {
            int prevL2Width = map(prevL2, 0, 255, 0, triggerWidth);
            if (prevL2Width > l2Width) {
                M5.Lcd.fillRect(leftX, y, prevL2Width, triggerHeight, TFT_BLACK);
            }
        }
        
        // Clear previous R2 if needed
        if (prevR2 >= 0) {
            int prevR2Width = map(prevR2, 0, 255, 0, triggerWidth);
            if (prevR2Width > r2Width) {
                M5.Lcd.fillRect(rightX, y, prevR2Width, triggerHeight, TFT_BLACK);
            }
        }
        
        // Draw L2 border and fill
        M5.Lcd.drawRect(leftX, y, triggerWidth, triggerHeight, TFT_DARKGREY);
        if (l2Width > 0) {
            M5.Lcd.fillRect(leftX, y, l2Width, triggerHeight, color);
        }
        
        // Draw R2 border and fill
        M5.Lcd.drawRect(rightX, y, triggerWidth, triggerHeight, TFT_DARKGREY);
        if (r2Width > 0) {
            M5.Lcd.fillRect(rightX, y, r2Width, triggerHeight, color);
        }
        
        // Label the triggers
        M5.Lcd.setTextSize(1);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextDatum(TC_DATUM);
        M5.Lcd.drawString("L2", leftX + triggerWidth/2, y - 10);
        M5.Lcd.drawString("R2", rightX + triggerWidth/2, y - 10);
    }
}

void ControllerScreen::drawShoulderButtons(int centerX, int centerY) {
    // Get channel values for L1 and R1 (assuming channels 4 and 5 for L1/R1)
    uint16_t l1Value = channelManager->getChannel(4);
    uint16_t r1Value = channelManager->getChannel(5);
    
    // Match dimensions with L2/R2 triggers
    const int buttonWidth = 30; // Same width as triggers
    const int buttonHeight = 8; // Same height as triggers
    const int BUTTON_SPACING = 5;
    
    // Calculate positions
    int leftX = centerX - buttonWidth - BUTTON_SPACING;
    int rightX = centerX + BUTTON_SPACING;
    
    // Calculate colors based on channel values
    uint16_t l1Color, r1Color;
    
    // Interpolate color from gray to green based on channel value
    if (l1Value <= CHANNEL_VALUE_MIN) {
        l1Color = TFT_DARKGREY;
    } else if (l1Value >= CHANNEL_VALUE_MAX) {
        l1Color = TFT_GREEN;
    } else {
        // Linear interpolation between colors
        uint8_t intensity = map(l1Value, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
        
        // Extract RGB components from TFT_DARKGREY (0x7BEF) and TFT_GREEN (0x07E0)
        uint8_t r1 = map(intensity, 0, 255, (TFT_DARKGREY >> 11) & 0x1F, (TFT_GREEN >> 11) & 0x1F);
        uint8_t g1 = map(intensity, 0, 255, (TFT_DARKGREY >> 5) & 0x3F, (TFT_GREEN >> 5) & 0x3F);
        uint8_t b1 = map(intensity, 0, 255, TFT_DARKGREY & 0x1F, TFT_GREEN & 0x1F);
        
        // Combine the components to get the interpolated color
        l1Color = (r1 << 11) | (g1 << 5) | b1;
    }
    
    // Same for R1
    if (r1Value <= CHANNEL_VALUE_MIN) {
        r1Color = TFT_DARKGREY;
    } else if (r1Value >= CHANNEL_VALUE_MAX) {
        r1Color = TFT_GREEN;
    } else {
        // Linear interpolation between colors
        uint8_t intensity = map(r1Value, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
        
        // Extract RGB components from TFT_DARKGREY (0x7BEF) and TFT_GREEN (0x07E0)
        uint8_t r1 = map(intensity, 0, 255, (TFT_DARKGREY >> 11) & 0x1F, (TFT_GREEN >> 11) & 0x1F);
        uint8_t g1 = map(intensity, 0, 255, (TFT_DARKGREY >> 5) & 0x3F, (TFT_GREEN >> 5) & 0x3F);
        uint8_t b1 = map(intensity, 0, 255, TFT_DARKGREY & 0x1F, TFT_GREEN & 0x1F);
        
        // Combine the components to get the interpolated color
        r1Color = (r1 << 11) | (g1 << 5) | b1;
    }
    
    // Draw L1 and R1 buttons with color coding (not as progress bars)
    M5.Lcd.fillRect(leftX, centerY, buttonWidth, buttonHeight, l1Color);
    M5.Lcd.fillRect(rightX, centerY, buttonWidth, buttonHeight, r1Color);
    
    // Add borders for better visibility
    M5.Lcd.drawRect(leftX, centerY, buttonWidth, buttonHeight, TFT_DARKGREY);
    M5.Lcd.drawRect(rightX, centerY, buttonWidth, buttonHeight, TFT_DARKGREY);
    
    // Draw L1/R1 labels - match label position style with triggers
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.drawString("L1", leftX + buttonWidth/2, centerY - 10);
    M5.Lcd.drawString("R1", rightX + buttonWidth/2, centerY - 10);
}

void ControllerScreen::drawButtons(int centerX, int centerY) {
    // Button size and spacing
    const int BUTTON_RADIUS = 8; // Increased by 2 pixels
    const int BUTTON_SPACING = 16; // Decreased by 2 pixels
    
    // Get button values from channelManager instead of controller
    uint16_t squareValue = channelManager->getChannel(10);    // Channel 10: Square
    uint16_t crossValue = channelManager->getChannel(8);      // Channel 8: Cross
    uint16_t circleValue = channelManager->getChannel(9);     // Channel 9: Circle
    uint16_t triangleValue = channelManager->getChannel(11);  // Channel 11: Triangle
    
    // Calculate colors based on channel values
    uint16_t squareColor, crossColor, circleColor, triangleColor;
    
    // For Square (Channel 10) - interpolate from dark gray to magenta
    if (squareValue <= CHANNEL_VALUE_MIN) {
        squareColor = TFT_DARKGREY;
    } else if (squareValue >= CHANNEL_VALUE_MAX) {
        squareColor = TFT_MAGENTA;
    } else {
        // Linear interpolation
        uint8_t intensity = map(squareValue, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
        // Extract and interpolate RGB components
        uint8_t r = map(intensity, 0, 255, (TFT_DARKGREY >> 11) & 0x1F, (TFT_MAGENTA >> 11) & 0x1F);
        uint8_t g = map(intensity, 0, 255, (TFT_DARKGREY >> 5) & 0x3F, (TFT_MAGENTA >> 5) & 0x3F);
        uint8_t b = map(intensity, 0, 255, TFT_DARKGREY & 0x1F, TFT_MAGENTA & 0x1F);
        squareColor = (r << 11) | (g << 5) | b;
    }
    
    // For Cross (Channel 8) - interpolate from dark gray to blue
    if (crossValue <= CHANNEL_VALUE_MIN) {
        crossColor = TFT_DARKGREY;
    } else if (crossValue >= CHANNEL_VALUE_MAX) {
        crossColor = TFT_BLUE;
    } else {
        uint8_t intensity = map(crossValue, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
        uint8_t r = map(intensity, 0, 255, (TFT_DARKGREY >> 11) & 0x1F, (TFT_BLUE >> 11) & 0x1F);
        uint8_t g = map(intensity, 0, 255, (TFT_DARKGREY >> 5) & 0x3F, (TFT_BLUE >> 5) & 0x3F);
        uint8_t b = map(intensity, 0, 255, TFT_DARKGREY & 0x1F, TFT_BLUE & 0x1F);
        crossColor = (r << 11) | (g << 5) | b;
    }
    
    // For Circle (Channel 9) - interpolate from dark gray to red
    if (circleValue <= CHANNEL_VALUE_MIN) {
        circleColor = TFT_DARKGREY;
    } else if (circleValue >= CHANNEL_VALUE_MAX) {
        circleColor = TFT_RED;
    } else {
        uint8_t intensity = map(circleValue, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
        uint8_t r = map(intensity, 0, 255, (TFT_DARKGREY >> 11) & 0x1F, (TFT_RED >> 11) & 0x1F);
        uint8_t g = map(intensity, 0, 255, (TFT_DARKGREY >> 5) & 0x3F, (TFT_RED >> 5) & 0x3F);
        uint8_t b = map(intensity, 0, 255, TFT_DARKGREY & 0x1F, TFT_RED & 0x1F);
        circleColor = (r << 11) | (g << 5) | b;
    }
    
    // For Triangle (Channel 11) - interpolate from dark gray to green
    if (triangleValue <= CHANNEL_VALUE_MIN) {
        triangleColor = TFT_DARKGREY;
    } else if (triangleValue >= CHANNEL_VALUE_MAX) {
        triangleColor = TFT_GREEN;
    } else {
        uint8_t intensity = map(triangleValue, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
        uint8_t r = map(intensity, 0, 255, (TFT_DARKGREY >> 11) & 0x1F, (TFT_GREEN >> 11) & 0x1F);
        uint8_t g = map(intensity, 0, 255, (TFT_DARKGREY >> 5) & 0x3F, (TFT_GREEN >> 5) & 0x3F);
        uint8_t b = map(intensity, 0, 255, TFT_DARKGREY & 0x1F, TFT_GREEN & 0x1F);
        triangleColor = (r << 11) | (g << 5) | b;
    }
    
    // Draw PS5-like button layout (circle pattern)
    // Triangle (top)
    M5.Lcd.fillCircle(centerX, centerY - BUTTON_SPACING, BUTTON_RADIUS, triangleColor);
    
    // Circle (right)
    M5.Lcd.fillCircle(centerX + BUTTON_SPACING, centerY, BUTTON_RADIUS, circleColor);
    
    // Cross (bottom)
    M5.Lcd.fillCircle(centerX, centerY + BUTTON_SPACING, BUTTON_RADIUS, crossColor);
    
    // Square (left)
    M5.Lcd.fillCircle(centerX - BUTTON_SPACING, centerY, BUTTON_RADIUS, squareColor);
    
    // Button symbols
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextDatum(MC_DATUM);
    
    // Draw triangle symbol
    M5.Lcd.setTextColor(triangleValue > CHANNEL_VALUE_MID ? TFT_BLACK : TFT_WHITE);
    M5.Lcd.drawString("^", centerX, centerY - BUTTON_SPACING);
    
    // Draw circle symbol
    M5.Lcd.setTextColor(circleValue > CHANNEL_VALUE_MID ? TFT_BLACK : TFT_WHITE);
    M5.Lcd.drawString("O", centerX + BUTTON_SPACING, centerY);
    
    // Draw cross symbol
    M5.Lcd.setTextColor(crossValue > CHANNEL_VALUE_MID ? TFT_BLACK : TFT_WHITE);
    M5.Lcd.drawString("X", centerX, centerY + BUTTON_SPACING);
    
    // Draw square symbol
    M5.Lcd.setTextColor(squareValue > CHANNEL_VALUE_MID ? TFT_BLACK : TFT_WHITE);
    M5.Lcd.drawString("□", centerX - BUTTON_SPACING, centerY);
}

void ControllerScreen::drawDPad(int centerX, int centerY) {
    // D-pad dimensions
    const int DPAD_SIZE = 15;
    const int DPAD_CROSS_SIZE = 10;
    
    // Get D-pad values from channelManager instead of controller
    uint16_t upValue = channelManager->getChannel(12);     // Channel 12: Up
    uint16_t downValue = channelManager->getChannel(13);   // Channel 13: Down
    uint16_t leftValue = channelManager->getChannel(14);   // Channel 14: Left
    uint16_t rightValue = channelManager->getChannel(15);  // Channel 15: Right
    
    // Calculate colors based on channel values
    uint16_t upColor, downColor, leftColor, rightColor;
    
    // Calculate color interpolation for UP button
    if (upValue <= CHANNEL_VALUE_MIN) {
        upColor = TFT_DARKGREY;
    } else if (upValue >= CHANNEL_VALUE_MAX) {
        upColor = TFT_WHITE;
    } else {
        uint8_t intensity = map(upValue, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
        uint8_t r = map(intensity, 0, 255, (TFT_DARKGREY >> 11) & 0x1F, (TFT_WHITE >> 11) & 0x1F);
        uint8_t g = map(intensity, 0, 255, (TFT_DARKGREY >> 5) & 0x3F, (TFT_WHITE >> 5) & 0x3F);
        uint8_t b = map(intensity, 0, 255, TFT_DARKGREY & 0x1F, TFT_WHITE & 0x1F);
        upColor = (r << 11) | (g << 5) | b;
    }
    
    // Calculate color interpolation for DOWN button
    if (downValue <= CHANNEL_VALUE_MIN) {
        downColor = TFT_DARKGREY;
    } else if (downValue >= CHANNEL_VALUE_MAX) {
        downColor = TFT_WHITE;
    } else {
        uint8_t intensity = map(downValue, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
        uint8_t r = map(intensity, 0, 255, (TFT_DARKGREY >> 11) & 0x1F, (TFT_WHITE >> 11) & 0x1F);
        uint8_t g = map(intensity, 0, 255, (TFT_DARKGREY >> 5) & 0x3F, (TFT_WHITE >> 5) & 0x3F);
        uint8_t b = map(intensity, 0, 255, TFT_DARKGREY & 0x1F, TFT_WHITE & 0x1F);
        downColor = (r << 11) | (g << 5) | b;
    }
    
    // Calculate color interpolation for LEFT button
    if (leftValue <= CHANNEL_VALUE_MIN) {
        leftColor = TFT_DARKGREY;
    } else if (leftValue >= CHANNEL_VALUE_MAX) {
        leftColor = TFT_WHITE;
    } else {
        uint8_t intensity = map(leftValue, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
        uint8_t r = map(intensity, 0, 255, (TFT_DARKGREY >> 11) & 0x1F, (TFT_WHITE >> 11) & 0x1F);
        uint8_t g = map(intensity, 0, 255, (TFT_DARKGREY >> 5) & 0x3F, (TFT_WHITE >> 5) & 0x3F);
        uint8_t b = map(intensity, 0, 255, TFT_DARKGREY & 0x1F, TFT_WHITE & 0x1F);
        leftColor = (r << 11) | (g << 5) | b;
    }
    
    // Calculate color interpolation for RIGHT button
    if (rightValue <= CHANNEL_VALUE_MIN) {
        rightColor = TFT_DARKGREY;
    } else if (rightValue >= CHANNEL_VALUE_MAX) {
        rightColor = TFT_WHITE;
    } else {
        uint8_t intensity = map(rightValue, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX, 0, 255);
        uint8_t r = map(intensity, 0, 255, (TFT_DARKGREY >> 11) & 0x1F, (TFT_WHITE >> 11) & 0x1F);
        uint8_t g = map(intensity, 0, 255, (TFT_DARKGREY >> 5) & 0x3F, (TFT_WHITE >> 5) & 0x3F);
        uint8_t b = map(intensity, 0, 255, TFT_DARKGREY & 0x1F, TFT_WHITE & 0x1F);
        rightColor = (r << 11) | (g << 5) | b;
    }
    
    // Draw D-pad cross base
    M5.Lcd.fillRect(centerX - DPAD_CROSS_SIZE/2, centerY - DPAD_SIZE, DPAD_CROSS_SIZE, DPAD_SIZE * 2, TFT_DARKGREY);
    M5.Lcd.fillRect(centerX - DPAD_SIZE, centerY - DPAD_CROSS_SIZE/2, DPAD_SIZE * 2, DPAD_CROSS_SIZE, TFT_DARKGREY);
    
    // Highlight active directions with interpolated colors
    // Up direction
    M5.Lcd.fillRect(centerX - DPAD_CROSS_SIZE/2, centerY - DPAD_SIZE, DPAD_CROSS_SIZE, DPAD_SIZE, upColor);
    
    // Down direction
    M5.Lcd.fillRect(centerX - DPAD_CROSS_SIZE/2, centerY, DPAD_CROSS_SIZE, DPAD_SIZE, downColor);
    
    // Left direction
    M5.Lcd.fillRect(centerX - DPAD_SIZE, centerY - DPAD_CROSS_SIZE/2, DPAD_SIZE, DPAD_CROSS_SIZE, leftColor);
    
    // Right direction
    M5.Lcd.fillRect(centerX, centerY - DPAD_CROSS_SIZE/2, DPAD_SIZE, DPAD_CROSS_SIZE, rightColor);
} 