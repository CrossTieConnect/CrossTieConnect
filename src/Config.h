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

// Configuration for button toggle behavior
#define DOUBLE_PRESS_TIME 500  // Time in ms to detect double press
#define CHANNEL_VALUE_MIN 172  // Min CRSF value
#define CHANNEL_VALUE_MAX 1811 // Max CRSF value
#define CHANNEL_VALUE_MID ((CHANNEL_VALUE_MIN + CHANNEL_VALUE_MAX) / 2)

// Long press duration
#define LONG_PRESS_DURATION 500 // Time in ms to detect long press

// Define CRSF pin and constants
#define CRSF_TX_PIN 26       // GPIO pin for CRSF (single-wire, half-duplex)
#define CRSF_BAUDRATE 420000 // CRSF standard baudrate
#define CRSF_PACKET_SIZE 64  // Maximum CRSF packet size
#define BIT_TIME_US (1000000 / CRSF_BAUDRATE)  // Microseconds per bit at CRSF baudrate

// CRSF specific defines
#define CRSF_SYNC_BYTE 0xC8
#define CRSF_FRAMETYPE_RC_CHANNELS_PACKED 0x16
#define CRSF_ADDRESS_FLIGHT_CONTROLLER 0xEE
#define CRSF_ADDRESS_CRSF_TRANSMITTER 0xEE
#define CRSF_ADDRESS_CRSF_RECEIVER 0xEC
#define CRSF_FRAME_SIZE 26   // Full frame size

// Line break/sync bytes for stream synchronization
#define SYNC_PREAMBLE_SIZE 8

// Debug LED pin (M5Stick's built-in LED)
#define DEBUG_LED_PIN 10

// Display dimensions and positions
#define STICK_SIZE 30
#define STICK_RADIUS 12
#define TRIGGER_HEIGHT 14
#define TRIGGER_WIDTH 60

// Left and right stick areas
#define LEFT_STICK_X 40
#define LEFT_STICK_Y 50
#define RIGHT_STICK_X 135
#define RIGHT_STICK_Y 50

// Trigger bar positions
#define LEFT_TRIGGER_X 20
#define RIGHT_TRIGGER_X 90
#define TRIGGERS_Y 95 