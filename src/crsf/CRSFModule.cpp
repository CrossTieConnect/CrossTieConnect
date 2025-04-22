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


#include "CRSFModule.h"
#include "../utils/Utils.h"

CRSFModule::CRSFModule(ChannelManager* channelManager) : 
    channelManager(channelManager),
    debugMode(false),
    lastUpdateTime(0) {
}

void CRSFModule::begin() {
    // Configure GPIO for output and set to idle state (inverted UART idle = LOW)
    pinMode(CRSF_TX_PIN, OUTPUT);
    digitalWrite(CRSF_TX_PIN, LOW);
    
    // Set up debug LED
    pinMode(DEBUG_LED_PIN, OUTPUT);
    digitalWrite(DEBUG_LED_PIN, LOW);
    
    Serial.println("CRSF software UART initialized on pin " + String(CRSF_TX_PIN));
}

void CRSFModule::update() {
    // Send CRSF packets at regular intervals (approximately 50Hz)
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= 20) {  // 50Hz update rate
        sendRcChannelsPacket();
        lastUpdateTime = currentTime;
    }
}

void CRSFModule::setDebugMode(bool debug) {
    debugMode = debug;
}

void CRSFModule::sendRcChannelsPacket() {
    uint8_t frame[CRSF_FRAME_SIZE];
    
    // Send sync preamble to help receiver synchronize
    sendSyncPreamble();
    
    // Add header
    frame[0] = CRSF_ADDRESS_FLIGHT_CONTROLLER;
    frame[1] = 24; // Length byte (payload + type + CRC = 22 + 1 + 1 = 24)
    frame[2] = CRSF_FRAMETYPE_RC_CHANNELS_PACKED;
    
    // Pack the channels into the payload
    packRcChannels(&frame[3], channelManager->getChannelData());
    
    // Calculate and add CRC
    frame[25] = crcCRSF(&frame[2], 23); // CRC over type + payload
    
    // Send the frame
    softUartSendBytes(frame, CRSF_FRAME_SIZE);
    
    // Debug output - show the channel values and first few bytes of the packed data
    if (debugMode) {
        static unsigned long lastDebugPrint = 0;
        if (millis() - lastDebugPrint > 500) { // Print only every 500ms to avoid flooding
            Serial.printf("CH0:%d CH1:%d CH2:%d CH3:%d | Data: %02X %02X %02X %02X\n", 
                        channelManager->getChannel(0), channelManager->getChannel(1), 
                        channelManager->getChannel(2), channelManager->getChannel(3),
                        frame[3], frame[4], frame[5], frame[6]);
            lastDebugPrint = millis();
        }
    }
}

void CRSFModule::packRcChannels(uint8_t *buffer, const uint16_t *channels) {
    // CRSF uses 11 bits per channel, packed across bytes
    
    // Channel 0: 8 bits in byte 0, 3 bits in byte 1
    buffer[0] = (channels[0] & 0xFF);
    buffer[1] = ((channels[0] >> 8) & 0x07);
    
    // Channel 1: 5 bits in byte 1, 6 bits in byte 2
    buffer[1] |= ((channels[1] & 0x1F) << 3);
    buffer[2] = ((channels[1] >> 5) & 0x3F);
    
    // Channel 2: 2 bits in byte 2, 8 bits in byte 3, 1 bit in byte 4
    buffer[2] |= ((channels[2] & 0x03) << 6);
    buffer[3] = ((channels[2] >> 2) & 0xFF);
    buffer[4] = ((channels[2] >> 10) & 0x01);
    
    // Channel 3: 7 bits in byte 4, 4 bits in byte 5
    buffer[4] |= ((channels[3] & 0x7F) << 1);
    buffer[5] = ((channels[3] >> 7) & 0x0F);
    
    // Channel 4: 4 bits in byte 5, 7 bits in byte 6
    buffer[5] |= ((channels[4] & 0x0F) << 4);
    buffer[6] = ((channels[4] >> 4) & 0x7F);
    
    // Channel 5: 1 bit in byte 6, 8 bits in byte 7, 2 bits in byte 8
    buffer[6] |= ((channels[5] & 0x01) << 7);
    buffer[7] = ((channels[5] >> 1) & 0xFF);
    buffer[8] = ((channels[5] >> 9) & 0x03);
    
    // Channel 6: 6 bits in byte 8, 5 bits in byte 9
    buffer[8] |= ((channels[6] & 0x3F) << 2);
    buffer[9] = ((channels[6] >> 6) & 0x1F);
    
    // Channel 7: 3 bits in byte 9, 8 bits in byte 10
    buffer[9] |= ((channels[7] & 0x07) << 5);
    buffer[10] = ((channels[7] >> 3) & 0xFF);
    
    // Channel 8: 0 bits in byte 10, 8 bits in byte 11, 3 bits in byte 12
    buffer[11] = (channels[8] & 0xFF);
    buffer[12] = ((channels[8] >> 8) & 0x07);
    
    // Channel 9: 5 bits in byte 12, 6 bits in byte 13
    buffer[12] |= ((channels[9] & 0x1F) << 3);
    buffer[13] = ((channels[9] >> 5) & 0x3F);
    
    // Channel 10: 2 bits in byte 13, 8 bits in byte 14, 1 bit in byte 15
    buffer[13] |= ((channels[10] & 0x03) << 6);
    buffer[14] = ((channels[10] >> 2) & 0xFF);
    buffer[15] = ((channels[10] >> 10) & 0x01);
    
    // Channel 11: 7 bits in byte 15, 4 bits in byte 16
    buffer[15] |= ((channels[11] & 0x7F) << 1);
    buffer[16] = ((channels[11] >> 7) & 0x0F);
    
    // Channel 12: 4 bits in byte 16, 7 bits in byte 17
    buffer[16] |= ((channels[12] & 0x0F) << 4);
    buffer[17] = ((channels[12] >> 4) & 0x7F);
    
    // Channel 13: 1 bit in byte 17, 8 bits in byte 18, 2 bits in byte 19
    buffer[17] |= ((channels[13] & 0x01) << 7);
    buffer[18] = ((channels[13] >> 1) & 0xFF);
    buffer[19] = ((channels[13] >> 9) & 0x03);
    
    // Channel 14: 6 bits in byte 19, 5 bits in byte 20
    buffer[19] |= ((channels[14] & 0x3F) << 2);
    buffer[20] = ((channels[14] >> 6) & 0x1F);
    
    // Channel 15: 3 bits in byte 20, 8 bits in byte 21
    buffer[20] |= ((channels[15] & 0x07) << 5);
    buffer[21] = ((channels[15] >> 3) & 0xFF);
}

void CRSFModule::sendSyncPreamble() {
    // Wiggle the line to help receiver synchronize
    const uint8_t preamble_bytes[4] = {0xFF, 0x00, 0xFF, 0x00};
    
    // Send preamble pattern
    softUartSendBytes(preamble_bytes, 4);
    delayMicroseconds(100);
}

void CRSFModule::softUartSendBytes(const uint8_t *data, size_t len) {
    // Toggle debug LED to indicate transmission
    digitalWrite(DEBUG_LED_PIN, HIGH);
    
    for (size_t i = 0; i < len; i++) {
        softUartSendByte(data[i]);
    }
    
    // Ensure line returns to idle state (inverted UART idle = LOW)
    digitalWrite(CRSF_TX_PIN, LOW);
    
    // Turn off debug LED
    digitalWrite(DEBUG_LED_PIN, LOW);
}

void CRSFModule::softUartSendByte(uint8_t data) {
    // Must be in IRAM for consistent timing
    noInterrupts();
    
    // Start bit (logical 0, inverted to HIGH)
    uartSendBit(false);
    
    // 8 data bits, LSB first
    for (int i = 0; i < 8; i++) {
        uartSendBit(data & (1 << i));
    }
    
    // Stop bit (logical 1, inverted to LOW)
    uartSendBit(true);
    
    // Extra delay for frame spacing
    uint32_t start = ESP.getCycleCount();
    uint32_t target = start + ((BIT_TIME_US / 2) * ESP.getCpuFreqMHz());
    
    while (ESP.getCycleCount() < target) {
        // Spacing delay
    }
    
    interrupts();
}

void CRSFModule::uartSendBit(bool bit_value) {
    // For inverted UART: true=LOW, false=HIGH
    digitalWrite(CRSF_TX_PIN, !bit_value);
    
    // Very precise delay
    uint32_t start = ESP.getCycleCount();
    uint32_t target = start + (BIT_TIME_US * ESP.getCpuFreqMHz());
    
    while (ESP.getCycleCount() < target) {
        // Tight busy-wait for precise timing
    }
} 