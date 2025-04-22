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


#include "ChannelManager.h"

ChannelManager::ChannelManager() {
    resetChannels();
    
    // Initialize previous values to match current
    for (int i = 0; i < NUM_CHANNELS; i++) {
        prevChannels[i] = channels[i];
    }
}

void ChannelManager::setChannel(uint8_t channel, uint16_t value) {
    if (channel < NUM_CHANNELS) {
        channels[channel] = value;
    }
}

uint16_t ChannelManager::getChannel(uint8_t channel) const {
    if (channel < NUM_CHANNELS) {
        return channels[channel];
    }
    return CHANNEL_VALUE_MID; // Return center value for invalid channels
}

const uint16_t* ChannelManager::getChannelData() const {
    return channels;
}

void ChannelManager::resetChannels() {
    // Set all channels to center position
    for (int i = 0; i < NUM_CHANNELS; i++) {
        channels[i] = CHANNEL_VALUE_MID;
    }
}

bool ChannelManager::hasChannelsChanged() {
    bool changed = false;
    
    // Check each channel for changes
    for (int i = 0; i < NUM_CHANNELS; i++) {
        if (channels[i] != prevChannels[i]) {
            changed = true;
            prevChannels[i] = channels[i]; // Update previous value
        }
    }
    
    return changed;
}

void ChannelManager::printChannels() const {
    Serial.println("Channel values:");
    for (int i = 0; i < NUM_CHANNELS; i++) {
        Serial.printf("CH%d: %d ", i, channels[i]);
        if ((i + 1) % 4 == 0) {
            Serial.println();
        }
    }
    Serial.println();
} 