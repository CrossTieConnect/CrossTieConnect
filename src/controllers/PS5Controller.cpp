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


#include "PS5Controller.h"
#include "../Config.h"
#include "../utils/Utils.h"
#include <Preferences.h>

PS5Controller::PS5Controller(ChannelManager* channelManager) : 
    Controller(channelManager),
    btnL1("L1", 2),
    btnR1("R1", 3),
    btnCross("Cross", 0),
    btnCircle("Circle", 0),
    btnSquare("Square", 0),
    btnTriangle("Triangle", 0),
    btnUp("Up", 0),
    btnDown("Down", 0),
    btnLeft("Left", 0),
    btnRight("Right", 0),
    btnL3("L3", 0),
    btnR3("R3", 0) {
    
    statusMessage = "WAITING";
    leftX = leftY = rightX = rightY = 0;
    l2Value = r2Value = 0;
    
    // Load MAC address from preferences or set initial state
    loadMacFromPreferences();
}

bool PS5Controller::begin() {
    // Force reload MAC address from preferences to ensure we have the latest
    loadMacFromPreferences();
    
    // Only attempt to connect if we have a MAC address
    if (macAddress.length() > 0) {
        // Initialize PS5 controller with the current MAC address
        ps5.begin(macAddress.c_str());
        Serial.println("PS5 Controller initialized, waiting for connection with MAC: " + macAddress);
        statusMessage = "WAITING";
        return true;
    } else {
        Serial.println("No MAC address set, please select a device from the connection screen");
        statusMessage = "NO MAC";
        return false;
    }
}

void PS5Controller::update() {
    bool wasConnected = connected;
    connected = ps5.isConnected();
    
    // Connection state changed
    if (connected != wasConnected) {
        if (connected) {
            statusMessage = "CONNECTED";
            Serial.println("PS5 Controller connected");
        } else {
            statusMessage = "DISCONNECTED";
            Serial.println("PS5 Controller disconnected");
            resetAllButtons();
        }
    }
    
    // Only update values when connected
    if (connected) {
        // Get analog inputs
        leftX = ps5.LStickX();
        leftY = ps5.LStickY();
        rightX = ps5.RStickX();
        rightY = ps5.RStickY();
        l2Value = ps5.L2Value();
        r2Value = ps5.R2Value();
        
        // Update button states
        btnL1.update(ps5.L1());
        btnR1.update(ps5.R1());
        btnCross.update(ps5.Cross());
        btnCircle.update(ps5.Circle());
        btnSquare.update(ps5.Square());
        btnTriangle.update(ps5.Triangle());
        btnUp.update(ps5.Up());
        btnDown.update(ps5.Down());
        btnLeft.update(ps5.Left());
        btnRight.update(ps5.Right());
        btnL3.update(ps5.L3());
        btnR3.update(ps5.R3());
        
        // Map controller values to channels
        mapControllerToChannels();
    }
}

bool PS5Controller::isConnected() const {
    return connected;
}

const char* PS5Controller::getStatusMessage() const {
    return statusMessage.c_str();
}

int PS5Controller::getAnalogValue(int index) const {
    switch (index) {
        case ANALOG_LEFT_X: return leftX;
        case ANALOG_LEFT_Y: return leftY;
        case ANALOG_RIGHT_X: return rightX;
        case ANALOG_RIGHT_Y: return rightY;
        case ANALOG_L2: return l2Value;
        case ANALOG_R2: return r2Value;
        default: return 0;
    }
}

bool PS5Controller::getButtonState(int index) const {
    switch (index) {
        case BUTTON_CROSS: return btnCross.getState() > 0;
        case BUTTON_CIRCLE: return btnCircle.getState() > 0;
        case BUTTON_SQUARE: return btnSquare.getState() > 0;
        case BUTTON_TRIANGLE: return btnTriangle.getState() > 0;
        case BUTTON_L1: return btnL1.getState() > 0;
        case BUTTON_R1: return btnR1.getState() > 0;
        case BUTTON_L3: return btnL3.getState() > 0;
        case BUTTON_R3: return btnR3.getState() > 0;
        case BUTTON_UP: return btnUp.getState() > 0;
        case BUTTON_DOWN: return btnDown.getState() > 0;
        case BUTTON_LEFT: return btnLeft.getState() > 0;
        case BUTTON_RIGHT: return btnRight.getState() > 0;
        default: return false;
    }
}

void PS5Controller::setButtonConfig(PS5Button button, int numStates) {
    switch (button) {
        case BUTTON_CROSS: btnCross.setNumStates(numStates); break;
        case BUTTON_CIRCLE: btnCircle.setNumStates(numStates); break;
        case BUTTON_SQUARE: btnSquare.setNumStates(numStates); break;
        case BUTTON_TRIANGLE: btnTriangle.setNumStates(numStates); break;
        case BUTTON_L1: btnL1.setNumStates(numStates); break;
        case BUTTON_R1: btnR1.setNumStates(numStates); break;
        case BUTTON_L3: btnL3.setNumStates(numStates); break;
        case BUTTON_R3: btnR3.setNumStates(numStates); break;
        case BUTTON_UP: btnUp.setNumStates(numStates); break;
        case BUTTON_DOWN: btnDown.setNumStates(numStates); break;
        case BUTTON_LEFT: btnLeft.setNumStates(numStates); break;
        case BUTTON_RIGHT: btnRight.setNumStates(numStates); break;
    }
}

void PS5Controller::resetAllButtons() {
    btnL1.reset();
    btnR1.reset();
    btnCross.reset();
    btnCircle.reset();
    btnSquare.reset();
    btnTriangle.reset();
    btnUp.reset();
    btnDown.reset();
    btnLeft.reset();
    btnRight.reset();
    btnL3.reset();
    btnR3.reset();
}

void PS5Controller::loadMacFromPreferences() {
    // Try to load MAC from preferences
    Preferences preferences;
    bool prefsOpened = false;
    
    // First try read-only mode
    prefsOpened = preferences.begin("ps5bridge", true);
    
    if (!prefsOpened) {
        // If read-only failed, try read-write which will create the namespace
        prefsOpened = preferences.begin("ps5bridge", false);
    }
    
    if (prefsOpened) {
        String savedMac = preferences.getString("mac", "");
        preferences.end();
        
        if (savedMac.length() > 0) {
            macAddress = savedMac;
            Serial.println("Loaded saved MAC address: " + macAddress);
        } else {
            // No saved MAC address - start with empty string to force connection screen
            macAddress = "";
            Serial.println("No saved MAC address, please select a device from connection screen");
        }
    } else {
        // If preferences couldn't be opened at all, start with empty string
        macAddress = "";
        Serial.println("Failed to open preferences, please select a device from connection screen");
    }
}

void PS5Controller::setMacAddress(const char* mac) {
    macAddress = mac;
    
    // Save to preferences for persistence
    Preferences preferences;
    if (preferences.begin("ps5bridge", false)) {
        preferences.putString("mac", macAddress);
        Serial.println("Saved MAC address to preferences: " + macAddress);
        preferences.end();
    } else {
        Serial.println("Failed to save MAC address to preferences");
    }
}

void PS5Controller::reconnect() {
    // Disconnect if already connected
    if (connected) {
        ps5.end();
        connected = false;
        statusMessage = "DISCONNECTED";
    }
    
    // Only attempt to reconnect if we have a MAC address
    if (macAddress.length() > 0) {
        ps5.begin(macAddress.c_str());
        statusMessage = "WAITING";
        Serial.println("Reconnecting to PS5 controller with MAC: " + macAddress);
    } else {
        statusMessage = "NO MAC";
        Serial.println("No MAC address set, cannot reconnect");
    }
}

void PS5Controller::mapControllerToChannels() {
    // Map sticks to channels 0-3
    channelManager->setChannel(0, mapValueClamped(leftX, -128, 127, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX));   // Channel 0: Left stick X
    channelManager->setChannel(1, mapValueClamped(rightY, -128, 127, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX));  // Channel 1: Right stick Y
    
    // FIX: Left stick Y (throttle) - Only uses positive range (0 to 127) 
    // Original mapping was treating up as max (127 → MAX) and center as min (-1 → MIN)
    channelManager->setChannel(2, mapValueClamped(leftY, 0, 127, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX));   // Channel 2: Left stick Y
    
    channelManager->setChannel(3, mapValueClamped(rightX, -128, 127, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX));  // Channel 3: Right stick X
    
    // Map buttons and triggers to channels 4-15
    channelManager->setChannel(4, btnL1.getValue());      // AUX1 (L1 button)
    channelManager->setChannel(5, btnR1.getValue());      // AUX2 (R1 button)
    channelManager->setChannel(6, mapValueClamped(l2Value, 0, 255, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX));  // AUX3 (L2 trigger)
    channelManager->setChannel(7, mapValueClamped(r2Value, 0, 255, CHANNEL_VALUE_MIN, CHANNEL_VALUE_MAX));  // AUX4 (R2 trigger)
    channelManager->setChannel(8, btnCross.getValue());   // AUX5 (Cross)
    channelManager->setChannel(9, btnCircle.getValue());  // AUX6 (Circle)
    channelManager->setChannel(10, btnSquare.getValue()); // AUX7 (Square)
    channelManager->setChannel(11, btnTriangle.getValue());// AUX8 (Triangle)
    channelManager->setChannel(12, btnUp.getValue());     // AUX9 (Up)
    channelManager->setChannel(13, btnDown.getValue());   // AUX10 (Down)
    channelManager->setChannel(14, btnLeft.getValue());   // AUX11 (Left)
    channelManager->setChannel(15, btnRight.getValue());  // AUX12 (Right)
} 