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

#include <ps5Controller.h>
#include "Controller.h"
#include "../utils/ButtonStateManager.h"

// Enum for PS5 controller analog inputs
enum PS5AnalogInput {
    ANALOG_LEFT_X = 0,
    ANALOG_LEFT_Y = 1,
    ANALOG_RIGHT_X = 2,
    ANALOG_RIGHT_Y = 3,
    ANALOG_L2 = 4,
    ANALOG_R2 = 5
};

// Enum for PS5 controller buttons
enum PS5Button {
    BUTTON_CROSS = 0,
    BUTTON_CIRCLE = 1,
    BUTTON_SQUARE = 2,
    BUTTON_TRIANGLE = 3,
    BUTTON_L1 = 4,
    BUTTON_R1 = 5,
    BUTTON_L3 = 6,
    BUTTON_R3 = 7,
    BUTTON_UP = 8,
    BUTTON_DOWN = 9,
    BUTTON_LEFT = 10,
    BUTTON_RIGHT = 11
};

class PS5Controller : public Controller {
public:
    PS5Controller(ChannelManager* channelManager);
    ~PS5Controller() override = default;
    
    // Initialize controller with MAC address
    bool begin() override;
    
    // Update controller state and channels
    void update() override;
    
    // Check if controller is connected
    bool isConnected() const override;
    
    // Get connection status message
    const char* getStatusMessage() const override;
    
    // Get analog value for display and debugging
    int getAnalogValue(int index) const override;
    
    // Get button state for display and debugging
    bool getButtonState(int index) const override;
    
    // Set button configuration (toggle/momentary)
    void setButtonConfig(PS5Button button, int numStates);
    
    // Reset all button states
    void resetAllButtons();

    // Set new MAC address and reconnect
    void setMacAddress(const char* mac);
    
    // Reconnect using the current MAC address
    void reconnect();

private:
    // Map controller inputs to channels
    void mapControllerToChannels();
    
    // Load MAC address from preferences
    void loadMacFromPreferences();
    
    // Status
    String statusMessage;
    
    // MAC address
    String macAddress;
    
    // Analog values
    int leftX, leftY, rightX, rightY, l2Value, r2Value;
    
    // Button state managers
    ButtonStateManager btnL1;
    ButtonStateManager btnR1;
    ButtonStateManager btnCross;
    ButtonStateManager btnCircle;
    ButtonStateManager btnSquare;
    ButtonStateManager btnTriangle;
    ButtonStateManager btnUp;
    ButtonStateManager btnDown;
    ButtonStateManager btnLeft;
    ButtonStateManager btnRight;
    ButtonStateManager btnL3;
    ButtonStateManager btnR3;
}; 