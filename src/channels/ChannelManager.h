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
#include "../Config.h"

// Number of RC channels to manage
#define NUM_CHANNELS 16

class ChannelManager {
public:
    ChannelManager();
    
    // Set channel value (index 0-15)
    void setChannel(uint8_t channel, uint16_t value);
    
    // Get channel value (index 0-15)
    uint16_t getChannel(uint8_t channel) const;
    
    // Get pointer to raw channel data
    const uint16_t* getChannelData() const;
    
    // Reset all channels to center/neutral position
    void resetChannels();
    
    // Check if any channels have changed since last check
    bool hasChannelsChanged();
    
    // For testing/debugging
    void printChannels() const;

private:
    uint16_t channels[NUM_CHANNELS]; // RC channel values
    uint16_t prevChannels[NUM_CHANNELS]; // Previous values for change detection
}; 