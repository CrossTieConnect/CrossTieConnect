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

#include <M5StickCPlus2.h>
#include <vector>
#include <string>
#include <ps5Controller.h>  // Direct include for PS5Controller
#include "Screen.h"
#include "../controllers/PS5Controller.h"

// Structure to hold Bluetooth device information
struct BluetoothDevice {
    std::string name;
    std::string address;
    bool isLikelyPS5;  // Flag to indicate if this is likely a PS5 controller
};

// Menu item types for ConnectionScreen
enum ConnectionMenuItem {
    ITEM_SCAN,        // Scan for devices
    ITEM_CLEAR,       // Clear saved MAC
    ITEM_SAVED_MAC,   // Saved MAC address entry
    ITEM_DEVICE       // Regular device entry
};

// Screen for Bluetooth device selection
class ConnectionScreen : public Screen {
public:
    ConnectionScreen(PS5Controller* controller);
    ~ConnectionScreen() override = default;
    
    // Screen interface
    void activate() override;
    void deactivate() override;
    void update() override;
    void handleButton(uint8_t button) override;
    
    // Start scanning for Bluetooth devices
    void startScan();
    
    // Connect to selected device
    void connectToSelected();
    
    // Navigate through the list
    void selectNext();
    void selectPrevious();
    
    // Add device to the list (called from discovery callback)
    void addDeviceToList(const String& name, const String& address);
    
    // Set if this is first activation (for auto-scanning on startup)
    void setFirstActivation(bool isFirst);
    
private:
    // Draw the screen content
    void drawScreen();
    
    // Save selected device to preferences
    void saveSelectedDevice();
    
    // Clear the saved MAC address
    void clearSavedMac();
    
    // Load saved MAC from preferences
    bool loadSavedMac(BluetoothDevice& device);
    
    // Check what type of menu item is selected
    ConnectionMenuItem getSelectedItemType() const;
    
    // Check if this is a likely PS5 controller by name/address
    bool isLikelyPS5Controller(const String& name, const String& address) const;
    
    // Calculate how many items can be displayed per page based on screen size
    int calculateItemsPerPage() const;
    
    PS5Controller* ps5Controller;
    std::vector<BluetoothDevice> devices;
    int selectedIndex;
    bool isScanning;
    unsigned long scanStartTime;
    bool useActiveScanning; // Flag to alternate between active and passive scanning
    bool isFirstActivation; // Flag to indicate if this is the first time the screen is activated
    bool hasSavedMac;       // Flag to indicate if there is a saved MAC
    
    // Constants
    static const int MAX_DEVICES = 50; // Increased from 10 to 50 to allow more devices to be found
    static const int SCAN_DURATION_MS = 10000; // 10 seconds for scan
    int currentPage;
    
    // Display parameters
    static const int ITEM_HEIGHT = 12;    // Height of a single line
    static const int ITEM_GAP = 8;        // Gap between items
    int itemsPerPage;                     // Dynamically calculated items per page
}; 