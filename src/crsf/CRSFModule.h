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
#include "../channels/ChannelManager.h"
#include "../Config.h"

class CRSFModule {
public:
    CRSFModule(ChannelManager* channelManager);
    
    // Initialize CRSF module
    void begin();
    
    // Update CRSF transmission
    void update();
    
    // Set debug mode (for serial output)
    void setDebugMode(bool debug);

private:
    // Methods for CRSF packet building and transmission
    void sendRcChannelsPacket();
    void packRcChannels(uint8_t *buffer, const uint16_t *channels);
    void sendSyncPreamble();
    
    // Methods for software UART transmission
    void softUartSendBytes(const uint8_t *data, size_t len);
    void softUartSendByte(uint8_t data);
    void uartSendBit(bool bit_value);
    
    ChannelManager* channelManager;
    bool debugMode;
    unsigned long lastUpdateTime;
}; 