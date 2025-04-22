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

#include "Screen.h"
#include "../controllers/Controller.h"
#include "../controllers/PS5Controller.h"
#include "../channels/ChannelManager.h"

class ControllerScreen : public Screen {
public:
    ControllerScreen(Controller* controller, ChannelManager* channelManager);
    ~ControllerScreen() override = default;
    
    void activate() override;
    void deactivate() override;
    void update() override;
    void handleButton(uint8_t button) override;
    
private:
    void drawStick(int x, int y, int valueX, int valueY, int prevX, int prevY, uint16_t color);
    void drawTrigger(int x, int y, int l2Value, int r2Value, int prevL2, int prevR2, uint16_t color);
    void drawButtons(int centerX, int centerY);
    void drawDPad(int centerX, int centerY);
    void drawShoulderButtons(int centerX, int centerY);
    
    Controller* controller;
    ChannelManager* channelManager;
    
    // Previous state tracking to reduce screen updates
    int prevLX, prevLY, prevRX, prevRY;
    int prevL2, prevR2;
}; 