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


#include "ConnectionScreen.h"
#include <Preferences.h>
#include <BluetoothSerial.h>
#include "ScreenManager.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. Please check ESP32 configuration.
#endif

// Forward declaration of global screenManager from main.cpp
extern ScreenManager screenManager;

// Bluetooth Serial instance - global and persistent for the app lifetime
static BluetoothSerial SerialBT;
static bool btInitialized = false;

// Empty callback for stopping discovery
static void empty_cb(BTAdvertisedDevice* device) {
    // Do nothing - just a placeholder for stopping discovery
}

// Variable to store the active ConnectionScreen instance for callback
static ConnectionScreen* activeConnectionScreen = nullptr;

// The Discovery callback (needs to match BTAdvertisedDeviceCb signature)
static void bt_discovery_cb(BTAdvertisedDevice* device) {
    // Use the global instance to add the device
    if (activeConnectionScreen) {
        // Get device info as Strings
        String name = device->haveName() ? device->getName().c_str() : "Unknown Device";
        String address = device->getAddress().toString().c_str();
        
        // Add to list via the ConnectionScreen method
        activeConnectionScreen->addDeviceToList(name, address);
    }
}

ConnectionScreen::ConnectionScreen(PS5Controller* controller) : 
    ps5Controller(controller),
    selectedIndex(0),
    isScanning(false),
    scanStartTime(0),
    useActiveScanning(true),
    isFirstActivation(true),
    hasSavedMac(false),
    currentPage(0) {
    
    // Calculate how many items can fit on the screen
    itemsPerPage = calculateItemsPerPage();
}

int ConnectionScreen::calculateItemsPerPage() const {
    // Calculate how many two-line items can fit on the screen
    // Each item needs 2 lines of ITEM_HEIGHT plus a gap of ITEM_GAP
    // Reserve space for title (2 lines) and bottom instruction (2 lines)
    const int reservedHeight = 4 * ITEM_HEIGHT;
    const int availableHeight = M5.Lcd.height() - reservedHeight;
    const int singleItemHeight = (2 * ITEM_HEIGHT) + ITEM_GAP;
    
    // Calculate how many complete items can fit
    int itemCount = availableHeight / singleItemHeight;
    
    // Ensure at least 1 item is shown
    return max(1, itemCount);
}

void ConnectionScreen::setFirstActivation(bool isFirst) {
    isFirstActivation = isFirst;
}

void ConnectionScreen::activate() {
    // Initialize display
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(1);
    
    // Clear device list first
    devices.clear();
    
    // Load saved MAC if available
    BluetoothDevice savedDevice;
    hasSavedMac = loadSavedMac(savedDevice);
    
    if (hasSavedMac) {
        // Add the saved device to the list
        devices.push_back(savedDevice);
    }
    
    // Reset selection to top of the list
    selectedIndex = 0;
    currentPage = 0;
    
    // Recalculate items per page in case the screen size has changed
    itemsPerPage = calculateItemsPerPage();
    
    // Auto-start scanning only if this is the first activation and there's no saved MAC
    if (isFirstActivation && !hasSavedMac) {
        startScan();
        isFirstActivation = false;
    }
    
    // Mark as needing redraw
    setNeedsRedraw();
}

void ConnectionScreen::deactivate() {
    try {
        // Stop scanning if in progress
        if (isScanning) {
            // Mark scanning as finished
            isScanning = false;
            activeConnectionScreen = nullptr;
            
            // Stop discovery by setting a 0-duration scan with empty callback
            SerialBT.discoverAsync(empty_cb, 0);
            
            Serial.println("Scan canceled during deactivation");
        }
    } catch (...) {
        Serial.println("Error in deactivate");
    }
}

void ConnectionScreen::update() {
    // Yield CPU time to prevent watchdog timeout
    yield();
    
    // Check if scan has timed out
    if (isScanning && millis() - scanStartTime > SCAN_DURATION_MS) {
        try {
            // Mark scanning as finished first
            isScanning = false;
            
            // Clear callback reference
            activeConnectionScreen = nullptr;
            
            // Stop discovery by setting a 0-duration scan with empty callback
            SerialBT.discoverAsync(empty_cb, 0);
            
            // Display message to user about connecting to the controllers
            Serial.println("Finished scan phase, found " + String(devices.size()) + " devices");
                
            // Update display to show results
            setNeedsRedraw();
            
        } catch (...) {
            // Catch any exceptions to prevent crashes
            Serial.println("Error stopping PS5 discovery");
            isScanning = false;
        }
    }
    
    // Yield some time back to the system to prevent watchdog timeouts
    delay(5);
    
    // Draw screen if needed
    if (needsRedraw()) {
        drawScreen();
    }
}

void ConnectionScreen::handleButton(uint8_t button) {
    // Not used in this class as button handling is done in main.cpp
}

bool ConnectionScreen::isLikelyPS5Controller(const String& name, const String& address) const {
    // Check name for keywords
    if (name.indexOf("DualSense") >= 0 || 
        name.indexOf("Wireless Controller") >= 0 ||
        name.indexOf("Sony") >= 0) {
        return true;
    }
    
    // Common PS5 controller MAC prefixes (convert to uppercase for matching)
    String upperAddress = address;
    upperAddress.toUpperCase();
    
    const char* ps5Prefixes[] = {"48:18", "3C:01", "58:FA", "28:99", "40:1C"};
    for (const char* prefix : ps5Prefixes) {
        if (upperAddress.startsWith(prefix)) {
            return true;
        }
    }
    
    return false;
}

// Add device to the list, helper method
void ConnectionScreen::addDeviceToList(const String& name, const String& address) {
    // Only process if still scanning
    if (!isScanning) {
        return;
    }
    
    // Convert Arduino String to std::string
    std::string stdName = name.c_str();
    std::string stdAddress = address.c_str();
    
    // Check if this is a likely PS5 controller
    bool isLikelyPS5 = isLikelyPS5Controller(name, address);
    
    // Check if this device is already in our list (only if still scanning)
    bool deviceExists = false;
    for (const auto& existingDevice : devices) {
        if (existingDevice.address == stdAddress) {
            deviceExists = true;
            break;
        }
    }
    
    if (!deviceExists) {
        // Create device entry
        BluetoothDevice btDevice;
        btDevice.name = stdName;
        btDevice.address = stdAddress;
        btDevice.isLikelyPS5 = isLikelyPS5;
        
        // Add to list based on type
        if (isLikelyPS5) {
            // Find the position after any saved device but before other devices
            auto it = devices.begin();
            
            // Skip the saved device if it exists
            if (hasSavedMac && !devices.empty()) {
                it++;
            }
            
            // Find the first non-PS5 device
            while (it != devices.end() && it->isLikelyPS5) {
                it++;
            }
            
            // Insert before that position
            devices.insert(it, btDevice);
        } else {
            devices.push_back(btDevice);
        }
        
        // Debug log
        Serial.printf("Found device: %s (%s) - %s\n", 
                    stdName.c_str(), 
                    stdAddress.c_str(), 
                    isLikelyPS5 ? "Likely PS5" : "Other");
        
        // Force redraw to show newly found device
        setNeedsRedraw();
    }
}

bool ConnectionScreen::loadSavedMac(BluetoothDevice& device) {
    Preferences preferences;
    if (preferences.begin("ps5bridge", true)) {
        String savedMac = preferences.getString("mac", "");
        preferences.end();
        
        if (savedMac.length() > 0) {
            device.name = "Saved Controller";
            device.address = savedMac.c_str();
            device.isLikelyPS5 = true;
            return true;
        }
    }
    return false;
}

void ConnectionScreen::clearSavedMac() {
    // Remove saved MAC from preferences
    Preferences preferences;
    if (preferences.begin("ps5bridge", false)) {
        preferences.remove("mac");
        preferences.end();
        Serial.println("Cleared saved MAC address");
    }
    
    // Update flags and display
    hasSavedMac = false;
    
    // Remove saved device from list if it exists
    if (!devices.empty() && devices[0].name == "Saved Controller") {
        devices.erase(devices.begin());
    }
    
    // Reset selection
    selectedIndex = 0;
    currentPage = 0;
    
    // Stop scanning if in progress
    if (isScanning) {
        isScanning = false;
        activeConnectionScreen = nullptr;
        SerialBT.discoverAsync(empty_cb, 0);
    }
    
    // Force redraw
    setNeedsRedraw();
}

void ConnectionScreen::startScan() {
    Serial.println("Starting Bluetooth Classic scan for PS5 controllers");
    
    // Make sure we're not scanning already
    if (isScanning) {
        // Stop scanning first
        isScanning = false;
        activeConnectionScreen = nullptr;
        // Stop any existing discovery
        SerialBT.discoverAsync(empty_cb, 0);
        delay(100);
    }
    
    // Initialize Bluetooth if needed
    if (!btInitialized) {
        if (SerialBT.begin("CrossTieConnectBridge", true)) {
            btInitialized = true;
            delay(100);  // Give time for BT to initialize
        } else {
            Serial.println("Failed to initialize Bluetooth");
            return;
        }
    }
    
    // We'll keep any saved device, but clear other discovered devices
    if (hasSavedMac && !devices.empty()) {
        BluetoothDevice savedDevice = devices[0];
        devices.clear();
        devices.push_back(savedDevice);
    } else {
        devices.clear();
    }
    
    // Set this as the active connection screen for callback
    activeConnectionScreen = this;
    
    // Start device discovery with callback (10 seconds timeout)
    bool scanStarted = SerialBT.discoverAsync(bt_discovery_cb, 10);
    
    if (!scanStarted) {
        Serial.println("Failed to start Bluetooth scan");
        return;
    }
    
    // Update status
    isScanning = true;
    scanStartTime = millis();
    
    // Reset selection to first device if available
    selectedIndex = 0;
    currentPage = 0;
    
    // Force redraw
    setNeedsRedraw();
}

ConnectionMenuItem ConnectionScreen::getSelectedItemType() const {
    // Calculate count of menu items before devices
    int menuItemCount = 1; // Scan button always exists
    if (hasSavedMac) {
        menuItemCount++; // Clear button
    }
    
    if (selectedIndex == 0) {
        return ITEM_SCAN;
    } else if (hasSavedMac && selectedIndex == 1) {
        return ITEM_CLEAR;
    } else if (hasSavedMac && selectedIndex == 2 && !devices.empty()) {
        return ITEM_SAVED_MAC;
    } else {
        return ITEM_DEVICE;
    }
}

void ConnectionScreen::connectToSelected() {
    ConnectionMenuItem itemType = getSelectedItemType();
    
    switch (itemType) {
        case ITEM_SCAN:
            startScan();
            return;
            
        case ITEM_CLEAR:
            clearSavedMac();
            return;
            
        case ITEM_SAVED_MAC:
        case ITEM_DEVICE: {
            // Calculate index into devices array based on menu structure
            int deviceIndex;
            if (hasSavedMac) {
                deviceIndex = selectedIndex - 2; // Adjust for Scan and Clear buttons
            } else {
                deviceIndex = selectedIndex - 1; // Adjust for Scan button only
            }
            
            // Check bounds
            if (deviceIndex < 0 || deviceIndex >= devices.size()) {
                return;
            }
            
            std::string address = devices[deviceIndex].address;
            
            // Check if this is a partial MAC address (ends with 00:00:00)
            if (address.find("00:00:00") != std::string::npos) {
                M5.Lcd.fillScreen(BLACK);
                M5.Lcd.setCursor(0, 10);
                M5.Lcd.setTextSize(2);
                M5.Lcd.setTextColor(RED);
                M5.Lcd.println("Warning: Partial MAC");
                M5.Lcd.println("Address detected!");
                M5.Lcd.println("");
                M5.Lcd.setTextColor(WHITE);
                M5.Lcd.println("Check your controller");
                M5.Lcd.println("for the full MAC addr:");
                M5.Lcd.setTextColor(YELLOW);
                M5.Lcd.println(address.substr(0, 8).c_str());
                M5.Lcd.println("XX:XX:XX");
                M5.Lcd.setTextColor(WHITE);
                M5.Lcd.println("");
                M5.Lcd.println("Edit Config.h with");
                M5.Lcd.println("the complete address");
                
                delay(5000); // Show this message for 5 seconds
                drawScreen();
                return;
            }
            
            // Save the MAC address and connect
            ps5Controller->setMacAddress(address.c_str());
            ps5Controller->reconnect();
            
            // Exit the connection screen and switch to status screen
            deactivate();
            
            // Switch to status screen to show connection progress
            screenManager.switchToScreen(SCREEN_STATUS);
            break;
        }
    }
}

void ConnectionScreen::drawScreen() {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(1);
    
    // Draw title
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println(" Bluetooth Menu");
    
    // Show scan status
    if (isScanning) {
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.print(" Scanning: ");
        M5.Lcd.print(String((SCAN_DURATION_MS - (millis() - scanStartTime)) / 1000));
        M5.Lcd.println("s");
    } else {
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.println("");
    }
    
    M5.Lcd.println();
    
    // Calculate total menu items
    int totalMenuItems = 1; // Scan button always exists
    if (hasSavedMac) {
        totalMenuItems++; // Clear button
    }
    totalMenuItems += devices.size(); // All devices
    
    // Calculate total pages
    int totalPages = (totalMenuItems + itemsPerPage - 1) / itemsPerPage;
    
    // Calculate items to show on current page
    int startIdx = currentPage * itemsPerPage;
    int endIdx = min(startIdx + itemsPerPage, totalMenuItems);
    
    // Draw menu items (2 lines per item)
    for (int i = startIdx; i < endIdx; i++) {
        bool isSelected = (i == selectedIndex);
        
        // Special menu items
        if (i == 0) {
            // Scan button
            if (isSelected) {
                M5.Lcd.fillRect(0, M5.Lcd.getCursorY(), M5.Lcd.width(), ITEM_HEIGHT, WHITE);
                M5.Lcd.setTextColor(BLACK, WHITE);
            } else {
                M5.Lcd.setTextColor(CYAN, BLACK);
            }
            
            M5.Lcd.println("[SCAN FOR DEVICES]");
            
            // Second line (empty)
            if (isSelected) {
                M5.Lcd.fillRect(0, M5.Lcd.getCursorY(), M5.Lcd.width(), ITEM_HEIGHT, WHITE);
            }
            M5.Lcd.println();
        } 
        else if (hasSavedMac && i == 1) {
            // Clear button
            if (isSelected) {
                M5.Lcd.fillRect(0, M5.Lcd.getCursorY(), M5.Lcd.width(), ITEM_HEIGHT, WHITE);
                M5.Lcd.setTextColor(BLACK, WHITE);
            } else {
                M5.Lcd.setTextColor(RED, BLACK);
            }
            
            M5.Lcd.println("[CLEAR SAVED DEVICE]");
            
            // Second line (empty)
            if (isSelected) {
                M5.Lcd.fillRect(0, M5.Lcd.getCursorY(), M5.Lcd.width(), ITEM_HEIGHT, WHITE);
            }
            M5.Lcd.println();
        }
        else {
            // Calculate device index based on menu structure
            int deviceIndex;
            if (hasSavedMac) {
                deviceIndex = i - 2; // Adjust for Scan and Clear buttons
            } else {
                deviceIndex = i - 1; // Adjust for Scan button only
            }
            
            // Check bounds
            if (deviceIndex < 0 || deviceIndex >= devices.size()) {
                continue;
            }
            
            // Regular device entry
            if (isSelected) {
                // Draw selection background for the first line
                M5.Lcd.fillRect(0, M5.Lcd.getCursorY(), M5.Lcd.width(), ITEM_HEIGHT, WHITE);
                M5.Lcd.setTextColor(BLACK, WHITE);
            } else {
                // Saved device in YELLOW, PS5 devices in WHITE, others in LIGHTGREY
                if (deviceIndex == 0 && hasSavedMac) {
                    M5.Lcd.setTextColor(YELLOW, BLACK);
                } else if (devices[deviceIndex].isLikelyPS5) {
                    M5.Lcd.setTextColor(WHITE, BLACK);
                } else {
                    M5.Lcd.setTextColor(LIGHTGREY, BLACK);
                }
            }
            
            // First line: Device name
            if (deviceIndex == 0 && hasSavedMac) {
                M5.Lcd.print("SAVED: ");
            }
            
            // Get the device name and trim it if needed to fit on screen
            String deviceName = devices[deviceIndex].name.c_str();
            
            // Calculate available width 
            // Each character is approximately 6 pixels wide in size 1
            int availableChars = M5.Lcd.width() / 6;
            
            // If this is the saved device with "SAVED: " prefix, reduce available chars
            if (deviceIndex == 0 && hasSavedMac) {
                availableChars -= 7; // Account for "SAVED: " (7 chars)
            }
            
            // Trim the name if it's too long
            if (deviceName.length() > availableChars) {
                deviceName = deviceName.substring(0, availableChars - 3) + "...";
            }
            
            M5.Lcd.print(deviceName);
            M5.Lcd.println();
            
            // Second line: MAC address
            if (isSelected) {
                // Draw selection background for the second line
                M5.Lcd.fillRect(0, M5.Lcd.getCursorY(), M5.Lcd.width(), ITEM_HEIGHT, WHITE);
                M5.Lcd.setTextColor(BLACK, WHITE);
            } else {
                M5.Lcd.setTextColor(BLUE, BLACK);
            }
            
            // Indent the MAC address
            M5.Lcd.print("  ");
            M5.Lcd.print(devices[deviceIndex].address.c_str());
            M5.Lcd.println();
        }
        
        // Add a small gap between items
        M5.Lcd.println();
    }
    
    // Show pagination indication if needed
    if (totalPages > 1) {
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.printf("Page %d/%d", currentPage + 1, totalPages);
        M5.Lcd.println();
    }
    
    // Show navigation instructions
    M5.Lcd.setCursor(0, M5.Lcd.height() - 20);
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.println("A: Select Option");
    M5.Lcd.println("B: Next | Long B: Prev");
    
    redrawNeeded = false;
}

void ConnectionScreen::selectNext() {
    // Calculate total menu items
    int totalMenuItems = 1; // Scan button always exists
    if (hasSavedMac) {
        totalMenuItems++; // Clear button
    }
    totalMenuItems += devices.size(); // All devices
    
    // Move to next item
    selectedIndex = (selectedIndex + 1) % totalMenuItems;
    
    // Update current page if needed
    int newPage = selectedIndex / itemsPerPage;
    if (newPage != currentPage) {
        currentPage = newPage;
    }
    
    setNeedsRedraw();
}

void ConnectionScreen::selectPrevious() {
    // Calculate total menu items
    int totalMenuItems = 1; // Scan button always exists
    if (hasSavedMac) {
        totalMenuItems++; // Clear button
    }
    totalMenuItems += devices.size(); // All devices
    
    // Move to previous item
    selectedIndex = (selectedIndex + totalMenuItems - 1) % totalMenuItems;
    
    // Update current page if needed
    int newPage = selectedIndex / itemsPerPage;
    if (newPage != currentPage) {
        currentPage = newPage;
    }
    
    setNeedsRedraw();
}

void ConnectionScreen::saveSelectedDevice() {
    ConnectionMenuItem itemType = getSelectedItemType();
    
    if (itemType == ITEM_DEVICE || itemType == ITEM_SAVED_MAC) {
        // Calculate device index based on menu structure
        int deviceIndex;
        if (hasSavedMac) {
            deviceIndex = selectedIndex - 2; // Adjust for Scan and Clear buttons
        } else {
            deviceIndex = selectedIndex - 1; // Adjust for Scan button only
        }
        
        // Check bounds
        if (deviceIndex >= 0 && deviceIndex < devices.size()) {
            Preferences preferences;
            if (preferences.begin("ps5bridge", false)) {
                preferences.putString("mac", devices[deviceIndex].address.c_str());
                preferences.end();
            }
        }
    }
} 