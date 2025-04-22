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

#include <Arduino.h>
#include <M5StickCPlus2.h>
#include <esp_task_wdt.h>  // Include for watchdog timer
#include <Preferences.h>   // Include for Preferences

#include "Config.h"
#include "channels/ChannelManager.h"
#include "controllers/PS5Controller.h"
#include "crsf/CRSFModule.h"
#include "display/ScreenManager.h"
#include "display/StatusScreen.h"
#include "display/ControllerScreen.h"
#include "display/LogoScreen.h"
#include "display/ConnectionScreen.h"

// Global objects
ChannelManager channelManager;
PS5Controller ps5Controller(&channelManager);
CRSFModule crsfModule(&channelManager);
ScreenManager screenManager;

// Connection screen reference for button handling
ConnectionScreen* connectionScreen = nullptr;

// Button handling
bool buttonAPressed = false;
bool buttonBPressed = false;
bool buttonBLongPressHandled = false;
unsigned long lastButtonCheck = 0;
bool wasPreviouslyConnected = false;
unsigned long startupTime = 0;
bool logoShown = false;

void checkButtons() {
    M5.update();
    
    // Check if we're on the connection screen to handle its specific button logic
    bool onConnectionScreen = (screenManager.getCurrentScreenType() == SCREEN_CONNECTION);
    
    // Track if Button A is being held for a long press
    static unsigned long btnAHoldStartTime = 0;
    static bool btnALongPressHandled = false;
    
    // Button A handling
    if (M5.BtnA.isPressed()) {
        // Button is currently pressed
        if (btnAHoldStartTime == 0) {
            // Just started pressing
            btnAHoldStartTime = millis();
            btnALongPressHandled = false;
        } else if (!btnALongPressHandled && (millis() - btnAHoldStartTime >= LONG_PRESS_DURATION)) {
            // Long press detected and not yet handled
            btnALongPressHandled = true;
            
            // Cycle through screens
            ScreenType currentScreen = screenManager.getCurrentScreenType();
            
            // Skip SCREEN_LOGO when cycling manually
            if (currentScreen == SCREEN_STATUS) {
                screenManager.switchToScreen(SCREEN_CONTROLLER);
            } else if (currentScreen == SCREEN_CONTROLLER) {
                screenManager.switchToScreen(SCREEN_CONNECTION);
            } else if (currentScreen == SCREEN_CONNECTION) {
                screenManager.switchToScreen(SCREEN_STATUS);
            } else if (currentScreen == SCREEN_LOGO) {
                // Skip logo screen and go directly to status
                screenManager.switchToScreen(SCREEN_STATUS);
                logoShown = true; // Mark logo as shown so we don't go back to it
            }
        }
    } else if (M5.BtnA.wasReleased()) {
        // Button was just released
        if (btnAHoldStartTime > 0 && !btnALongPressHandled && onConnectionScreen && connectionScreen) {
            // This was a short press since long press wasn't handled
            connectionScreen->connectToSelected();
        }
        // Reset the tracking variables
        btnAHoldStartTime = 0;
    }
    
    // Handle Button B for connection screen 
    if (onConnectionScreen && connectionScreen) {
        // Track long press state to avoid double-triggering
        if (M5.BtnB.isHolding()) {
            // Button B is being held down
            if (!buttonBLongPressHandled && millis() - M5.BtnB.lastChange() >= LONG_PRESS_DURATION) {
                // This is a long press and we haven't handled it yet
                connectionScreen->selectPrevious();
                buttonBLongPressHandled = true;
            }
        } else if (M5.BtnB.wasPressed()) {
            // Short press (new press) - only handle if no long press was handled
            buttonBLongPressHandled = false;
        } else if (M5.BtnB.wasReleased()) {
            // Button was released - if no long press was handled, treat as short press
            if (!buttonBLongPressHandled) {
                connectionScreen->selectNext();
            }
            // Reset the long press handled flag
            buttonBLongPressHandled = false;
        }
    }
}

// Check if PS5 controller has a saved MAC address
bool hasSavedMacAddress() {
    String savedMac = "";
    Preferences preferences;
    bool prefsOpened = false;
    
    // First try read-only mode
    prefsOpened = preferences.begin("ps5bridge", true);
    
    if (!prefsOpened) {
        // If read-only failed, try read-write which will create the namespace
        prefsOpened = preferences.begin("ps5bridge", false);
    }
    
    if (prefsOpened) {
        savedMac = preferences.getString("mac", "");
        preferences.end();
    }
    
    return savedMac.length() > 0;
}

void setup() {
  // Initialize M5StickCPlus2
  M5.begin();
  
  // Initialize serial
  Serial.begin(115200);
  
  // Record startup time
  startupTime = millis();
  
  Serial.println("Starting PS5 to CRSF Bridge");
  
  // Increase task watchdog timeout to prevent crashes during BLE scanning
  // This is especially needed for Bluetooth operations which can be intensive
  esp_task_wdt_init(10, true); // 10 second timeout, panic on timeout
  
  // Set up screens
  LogoScreen* logoScreen = new LogoScreen();
  StatusScreen* statusScreen = new StatusScreen(&ps5Controller);
  ControllerScreen* controllerScreen = new ControllerScreen(&ps5Controller, &channelManager);
  connectionScreen = new ConnectionScreen(&ps5Controller);
  
  // Mark the connection screen for first-time activation
  // This will trigger auto-scanning if no saved MAC is found
  connectionScreen->setFirstActivation(true);
  
  // Register screens with manager
  screenManager.registerScreen(SCREEN_LOGO, logoScreen);
  screenManager.registerScreen(SCREEN_STATUS, statusScreen);
  screenManager.registerScreen(SCREEN_CONTROLLER, controllerScreen);
  screenManager.registerScreen(SCREEN_CONNECTION, connectionScreen);
  
  // Start with logo screen
  screenManager.switchToScreen(SCREEN_LOGO);
  
  // Reset channels to center position
  channelManager.resetChannels();
  
  // Initialize PS5 controller (but don't start search automatically)
  // It will connect if a MAC address is saved
  ps5Controller.begin();
}

void loop() {
  // Check if it's time to initialize CRSF and switch from logo
  if (!logoShown && (millis() - startupTime > 2000)) {
    // Initialize CRSF module after 2 second delay
    crsfModule.begin();
    
    // Check if we have a saved MAC address
    bool hasMac = hasSavedMacAddress();
    
    if (hasMac) {
      // We have a saved MAC address, go to status screen
      screenManager.switchToScreen(SCREEN_STATUS);
    } else {
      // No saved MAC address, go to connection screen
      screenManager.switchToScreen(SCREEN_CONNECTION);
    }
    
    logoShown = true;
  }
  
  // Update controller (reads inputs and updates channels)
  ps5Controller.update();
  
  // Update CRSF transmission
  crsfModule.update();
  
  // Auto-switch screens based on controller connection status
  // Only after logo screen has been shown and not on connection screen
  if (logoShown && screenManager.getCurrentScreenType() != SCREEN_CONNECTION) {
    bool isConnected = ps5Controller.isConnected();
    if (isConnected != wasPreviouslyConnected) {
      // Connection state changed
      if (isConnected) {
        // Controller connected - switch to controller screen
        screenManager.switchToScreen(SCREEN_CONTROLLER);
      } else {
        // Controller disconnected - switch to status screen
        screenManager.switchToScreen(SCREEN_STATUS);
      }
      wasPreviouslyConnected = isConnected;
    }
  }
  
  // Update display
  screenManager.update();
  
  // Check buttons every 100ms
  unsigned long currentTime = millis();
  if (currentTime - lastButtonCheck > 100) {
    checkButtons();
    lastButtonCheck = currentTime;
  }
  
  // Small delay to prevent hogging CPU
  delay(1);
} 