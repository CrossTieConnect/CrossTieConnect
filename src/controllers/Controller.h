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

#include "../channels/ChannelManager.h"

// Abstract base class for all controllers
class Controller {
public:
    Controller(ChannelManager* channelManager);
    virtual ~Controller() = default;
    
    // Initialize controller
    virtual bool begin() = 0;
    
    // Update controller state and channels
    virtual void update() = 0;
    
    // Check if controller is connected
    virtual bool isConnected() const = 0;
    
    // Get connection status message
    virtual const char* getStatusMessage() const = 0;
    
    // Get analog value for display and debugging
    virtual int getAnalogValue(int index) const = 0;
    
    // Get button state for display and debugging
    virtual bool getButtonState(int index) const = 0;

protected:
    ChannelManager* channelManager;
    bool connected;
}; 