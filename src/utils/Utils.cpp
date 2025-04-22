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


#include "Utils.h"

// Convert a value from one range to another with clamping
int mapValueClamped(int value, int from_min, int from_max, int to_min, int to_max) {
    // Clamp the value to the range of the input
    if (value < from_min) value = from_min;
    if (value > from_max) value = from_max;
    
    // Map the value to the range of the output
    int result = (value - from_min) * (to_max - to_min) / (from_max - from_min) + to_min;
    
    // Clamp the result to ensure it stays within to_min and to_max
    if (result < to_min) result = to_min;
    if (result > to_max) result = to_max;
    
    return result;
}

// CRC8-DVB-S2 as used in the CRSF protocol
uint8_t crcCRSF(const uint8_t *buf, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0xD5;
            } else {
                crc = crc << 1;
            }
        }
    }
    return crc;
} 