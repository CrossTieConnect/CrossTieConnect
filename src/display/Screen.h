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

// Abstract base class for all screens
class Screen {
public:
    Screen();
    virtual ~Screen() = default;
    
    // Called when screen becomes active
    virtual void activate() = 0;
    
    // Called when screen becomes inactive
    virtual void deactivate() = 0;
    
    // Update screen content
    virtual void update() = 0;
    
    // Process button input
    virtual void handleButton(uint8_t button) = 0;
    
    // Check if screen needs to be redrawn
    bool needsRedraw() const;
    
    // Mark screen as needing redraw
    void setNeedsRedraw();
    
protected:
    bool redrawNeeded;
}; 