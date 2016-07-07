//
//  VoodooElanTouchpadDevice.cpp
//  VoodooI2C
//
//  Created by CoolStar on 12/13/15.
//  Copyright © 2015 CoolStar. All rights reserved.
//  ported from crostrackpad-elan 3.0 beta 9.4 for Windows
//

#include "VoodooElanTouchpadDevice.h"
#include "VoodooI2C.h"
#include "VoodooElanTouchpadWrapper.h"

OSDefineMetaClassAndStructors(VoodooI2CElanTouchpadDevice, VoodooI2CDevice);

#ifndef ABS32
#define ABS32
inline int32_t abs(int32_t num){
    if (num < 0){
        return num * -1;
    }
    return num;
}
#endif

typedef unsigned char BYTE;

#define REPORTID_FEATURE        0x02
#define REPORTID_RELATIVE_MOUSE 0x04
#define REPORTID_TOUCHPAD       0x05
#define REPORTID_KEYBOARD       0x07

#define MOUSE_BUTTON_1     0x01
#define MOUSE_BUTTON_2     0x02
#define MOUSE_BUTTON_3     0x04

#define KBD_LCONTROL_BIT     1
#define KBD_LGUI_BIT         8

#define KBD_KEY_CODES        6

unsigned char elandesc[] = {
    //
    // Relative mouse report starts here
    //
    0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                         // USAGE (Mouse)
    0xa1, 0x01,                         // COLLECTION (Application)
    0x85, REPORTID_RELATIVE_MOUSE,      //   REPORT_ID (Mouse)
    0x09, 0x01,                         //   USAGE (Pointer)
    0xa1, 0x00,                         //   COLLECTION (Physical)
    0x05, 0x09,                         //     USAGE_PAGE (Button)
    0x19, 0x01,                         //     USAGE_MINIMUM (Button 1)
    0x29, 0x05,                         //     USAGE_MAXIMUM (Button 5)
    0x15, 0x00,                         //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                         //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,                         //     REPORT_SIZE (1)
    0x95, 0x05,                         //     REPORT_COUNT (5)
    0x81, 0x02,                         //     INPUT (Data,Var,Abs)
    0x95, 0x03,                         //     REPORT_COUNT (3)
    0x81, 0x03,                         //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                         //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                         //     USAGE (X)
    0x09, 0x31,                         //     USAGE (Y)
    0x15, 0x81,                         //     Logical Minimum (-127)
    0x25, 0x7F,                         //     Logical Maximum (127)
    0x75, 0x08,                         //     REPORT_SIZE (8)
    0x95, 0x02,                         //     REPORT_COUNT (2)
    0x81, 0x06,                         //     INPUT (Data,Var,Rel)
    0x05, 0x01,                         //     Usage Page (Generic Desktop)
    0x09, 0x38,                         //     Usage (Wheel)
    0x15, 0x81,                         //     Logical Minimum (-127)
    0x25, 0x7F,                         //     Logical Maximum (127)
    0x75, 0x08,                         //     Report Size (8)
    0x95, 0x01,                         //     Report Count (1)
    0x81, 0x06,                         //     Input (Data, Variable, Relative)
    // ------------------------------  Horizontal wheel
    0x05, 0x0c,                         //     USAGE_PAGE (Consumer Devices)
    0x0a, 0x38, 0x02,                   //     USAGE (AC Pan)
    0x15, 0x81,                         //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                         //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                         //     REPORT_SIZE (8)
    0x95, 0x01,                         //     Report Count (1)
    0x81, 0x06,                         //     Input (Data, Variable, Relative)
    0xc0,                               //   END_COLLECTION
    0xc0,                               // END_COLLECTION
    
    /*//TOUCH PAD input TLC
     0x05, 0x0d,                         // USAGE_PAGE (Digitizers)
     0x09, 0x05,                         // USAGE (Touch Pad)
     0xa1, 0x01,                         // COLLECTION (Application)
     0x85, REPORTID_TOUCHPAD,            //   REPORT_ID (Touch pad)
     0x09, 0x22,                         //   USAGE (Finger)
     0xa1, 0x02,                         //   COLLECTION (Logical)
     0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
     0x25, 0x01,                         //       LOGICAL_MAXIMUM (1)
     0x09, 0x47,                         //       USAGE (Confidence)
     0x09, 0x42,                         //       USAGE (Tip switch)
     0x95, 0x02,                         //       REPORT_COUNT (2)
     0x75, 0x01,                         //       REPORT_SIZE (1)
     0x81, 0x02,                         //       INPUT (Data,Var,Abs)
     0x95, 0x01,                         //       REPORT_COUNT (1)
     0x75, 0x02,                         //       REPORT_SIZE (2)
     0x25, 0x02,                         //       LOGICAL_MAXIMUM (2)
     0x09, 0x51,                         //       USAGE (Contact Identifier)
     0x81, 0x02,                         //       INPUT (Data,Var,Abs)
     0x75, 0x01,                         //       REPORT_SIZE (1)
     0x95, 0x04,                         //       REPORT_COUNT (4)
     0x81, 0x03,                         //       INPUT (Cnst,Var,Abs)
     0x05, 0x01,                         //       USAGE_PAGE (Generic Desk..
     0x15, 0x00,                         //       LOGICAL_MINIMUM (0)
     0x26, 0xff, 0x0f,                   //       LOGICAL_MAXIMUM (4095)
     0x75, 0x10,                         //       REPORT_SIZE (16)
     0x55, 0x0e,                         //       UNIT_EXPONENT (-2)
     0x65, 0x13,                         //       UNIT(Inch,EngLinear)
     0x09, 0x30,                         //       USAGE (X)
     0x35, 0x00,                         //       PHYSICAL_MINIMUM (0)
     0x46, 0x90, 0x01,                   //       PHYSICAL_MAXIMUM (400)
     0x95, 0x01,                         //       REPORT_COUNT (1)
     0x81, 0x02,                         //       INPUT (Data,Var,Abs)
     0x46, 0x13, 0x01,                   //       PHYSICAL_MAXIMUM (275)
     0x09, 0x31,                         //       USAGE (Y)
     0x81, 0x02,                         //       INPUT (Data,Var,Abs)
     0xc0,                               //    END_COLLECTION
     0xc0,                               // END_COLLECTION*/
    
    //
    // Keyboard report starts here
    //
     0x05, 0x01,                         // USAGE_PAGE (Generic Desktop)
     0x09, 0x06,                         // USAGE (Keyboard)
     0xa1, 0x01,                         // COLLECTION (Application)
     0x85, REPORTID_KEYBOARD,            //   REPORT_ID (Keyboard)
     0x05, 0x07,                         //   USAGE_PAGE (Keyboard)
     0x19, 0xe0,                         //   USAGE_MINIMUM (Keyboard LeftControl)
     0x29, 0xe7,                         //   USAGE_MAXIMUM (Keyboard Right GUI)
     0x15, 0x00,                         //   LOGICAL_MINIMUM (0)
     0x25, 0x01,                         //   LOGICAL_MAXIMUM (1)
     0x75, 0x01,                         //   REPORT_SIZE (1)
     0x95, 0x08,                         //   REPORT_COUNT (8)
     0x81, 0x02,                         //   INPUT (Data,Var,Abs)
     0x95, 0x01,                         //   REPORT_COUNT (1)
     0x75, 0x08,                         //   REPORT_SIZE (8)
     0x81, 0x03,                         //   INPUT (Cnst,Var,Abs)
     0x95, 0x05,                         //   REPORT_COUNT (5)
     0x75, 0x01,                         //   REPORT_SIZE (1)
     0x05, 0x08,                         //   USAGE_PAGE (LEDs)
     0x19, 0x01,                         //   USAGE_MINIMUM (Num Lock)
     0x29, 0x05,                         //   USAGE_MAXIMUM (Kana)
     0x91, 0x02,                         //   OUTPUT (Data,Var,Abs)
     0x95, 0x01,                         //   REPORT_COUNT (1)
     0x75, 0x03,                         //   REPORT_SIZE (3)
     0x91, 0x03,                         //   OUTPUT (Cnst,Var,Abs)
     0x95, 0x06,                         //   REPORT_COUNT (6)
     0x75, 0x08,                         //   REPORT_SIZE (8)
     0x15, 0x00,                         //   LOGICAL_MINIMUM (0)
     0x25, 0x65,                         //   LOGICAL_MAXIMUM (101)
     0x05, 0x07,                         //   USAGE_PAGE (Keyboard)
     0x19, 0x00,                         //   USAGE_MINIMUM (Reserved (no event indicated))
     0x29, 0x65,                         //   USAGE_MAXIMUM (Keyboard Application)
     0x81, 0x00,                         //   INPUT (Data,Ary,Abs)
     0xc0,                               // END_COLLECTION
};

typedef struct  __attribute__((__packed__)) _ELAN_RELATIVE_MOUSE_REPORT
{
    
    UInt8        ReportID;
    
    UInt8        Button;
    
    UInt8        XValue;
    
    UInt8        YValue;
    
    UInt8        WheelPosition;
    
    UInt8		HWheelPosition;
    
} ElanRelativeMouseReport;

typedef struct __attribute__((__packed__)) _ELAN_KEYBOARD_REPORT
{
    
    BYTE      ReportID;
    
    // Left Control, Left Shift, Left Alt, Left GUI
    // Right Control, Right Shift, Right Alt, Right GUI
    BYTE      ShiftKeyFlags;
    
    BYTE      Reserved;
    
    // See http://www.usb.org/developers/devclass_docs/Hut1_11.pdf
    // for a list of key codes
    BYTE      KeyCodes[KBD_KEY_CODES];
    
} ElanKeyboardReport;

int VoodooI2CElanTouchpadDevice::distancesq(int delta_x, int delta_y){
    return (delta_x * delta_x) + (delta_y*delta_y);
}

bool VoodooI2CElanTouchpadDevice::ProcessMove(csgesture_softc *sc, int abovethreshold, int iToUse[3]) {
    if (abovethreshold == 1 || sc->panningActive) {
        int i = iToUse[0];
        if (!sc->panningActive && sc->tick[i] < 5)
            return false;
        
        if (sc->panningActive && i == -1)
            i = sc->idForPanning;
        
        int delta_x = sc->x[i] - sc->lastx[i];
        int delta_y = sc->y[i] - sc->lasty[i];
        
        if (abs(delta_x) > 75 || abs(delta_y) > 75) {
            delta_x = 0;
            delta_y = 0;
        }
        
        for (int j = 0;j < MAX_FINGERS;j++) {
            if (j != i) {
                if (sc->blacklistedids[j] != 1) {
                    if (sc->y[j] > sc->y[i]) {
                        if (sc->truetick[j] > sc->truetick[i] + 15) {
                            sc->blacklistedids[j] = 1;
                        }
                    }
                }
            }
        }
        
        sc->dx = delta_x;
        sc->dy = delta_y;
        
        sc->panningActive = true;
        sc->idForPanning = i;
        return true;
    }
    return false;
}

bool VoodooI2CElanTouchpadDevice::ProcessScroll(csgesture_softc *sc, int abovethreshold, int iToUse[3]) {
    sc->scrollx = 0;
    sc->scrolly = 0;
    if (abovethreshold == 2 || sc->scrollingActive) {
        int i1 = iToUse[0];
        int i2 = iToUse[1];
        if (sc->scrollingActive){
            if (i1 == -1) {
                if (i2 != sc->idsForScrolling[0])
                    i1 = sc->idsForScrolling[0];
                else
                    i1 = sc->idsForScrolling[1];
            }
            if (i2 == -1) {
                if (i1 != sc->idsForScrolling[0])
                    i2 = sc->idsForScrolling[0];
                else
                    i2 = sc->idsForScrolling[1];
            }
        }
        
        int delta_x1 = sc->x[i1] - sc->lastx[i1];
        int delta_y1 = sc->y[i1] - sc->lasty[i1];
        
        int delta_x2 = sc->x[i2] - sc->lastx[i2];
        int delta_y2 = sc->y[i2] - sc->lasty[i2];
        
        if ((abs(delta_y1) + abs(delta_y2)) > (abs(delta_x1) + abs(delta_x2))) {
            int avgy = (delta_y1 + delta_y2) / 2;
            sc->scrolly = avgy;
        }
        else {
            int avgx = (delta_x1 + delta_x2) / 2;
            sc->scrollx = avgx;
        }
        if (abs(sc->scrollx) > 100)
            sc->scrollx = 0;
        if (abs(sc->scrolly) > 100)
            sc->scrolly = 0;
        if (sc->scrolly > 8)
            sc->scrolly = sc->scrolly / 8;
        else if (sc->scrolly > 5)
            sc->scrolly = 1;
        else if (sc->scrolly < -8)
            sc->scrolly = sc->scrolly / 8;
        else if (sc->scrolly < -5)
            sc->scrolly = -1;
        else
            sc->scrolly = 0;
        
        if (sc->scrollx > 8) {
            sc->scrollx = sc->scrollx / 8;
            sc->scrollx = -sc->scrollx;
        }
        else if (sc->scrollx > 5)
            sc->scrollx = -1;
        else if (sc->scrollx < -8) {
            sc->scrollx = sc->scrollx / 8;
            sc->scrollx = -sc->scrollx;
        }
        else if (sc->scrollx < -5)
            sc->scrollx = 1;
        else
            sc->scrollx = 0;
        
        sc->scrollx = -sc->scrollx;
        sc->scrolly = -sc->scrolly;
        
        int fngrcount = 0;
        int totfingers = 0;
        for (int i = 0; i < MAX_FINGERS; i++) {
            if (sc->x[i] != -1) {
                totfingers++;
                if (i == i1 || i == i2)
                    fngrcount++;
            }
        }
        
        if (fngrcount == 2)
            sc->ticksSinceScrolling = 0;
        else
            sc->ticksSinceScrolling++;
        if (fngrcount == 2 || sc->ticksSinceScrolling <= 5) {
            sc->scrollingActive = true;
            if (abovethreshold == 2){
                sc->idsForScrolling[0] = iToUse[0];
                sc->idsForScrolling[1] = iToUse[1];
            }
        }
        else {
            sc->scrollingActive = false;
            sc->idsForScrolling[0] = -1;
            sc->idsForScrolling[1] = -1;
        }
        return true;
    }
    return false;
}

bool VoodooI2CElanTouchpadDevice::ProcessThreeFingerSwipe(csgesture_softc *sc, int abovethreshold, int iToUse[3]) {
    if (abovethreshold == 3 || abovethreshold == 4) {
        int i1 = iToUse[0];
        int delta_x1 = sc->x[i1] - sc->lastx[i1];
        int delta_y1 = sc->y[i1] - sc->lasty[i1];
        
        int i2 = iToUse[1];
        int delta_x2 = sc->x[i2] - sc->lastx[i2];
        int delta_y2 = sc->y[i2] - sc->lasty[i2];
        
        int i3 = iToUse[2];
        int delta_x3 = sc->x[i3] - sc->lastx[i3];
        int delta_y3 = sc->y[i3] - sc->lasty[i3];
        
        int avgx = (delta_x1 + delta_x2 + delta_x3) / 3;
        int avgy = (delta_y1 + delta_y2 + delta_y3) / 3;
        
        sc->multitaskingx += avgx;
        sc->multitaskingy += avgy;
        sc->multitaskinggesturetick++;
        
        if (sc->multitaskinggesturetick > 5 && !sc->multitaskingdone) {
            if ((abs(delta_y1) + abs(delta_y2) + abs(delta_y3)) > (abs(delta_x1) + abs(delta_x2) + abs(delta_x3))) {
                if (abs(sc->multitaskingy) > 50) {
                    uint8_t shiftKeys = KBD_LCONTROL_BIT;
                    uint8_t keyCodes[KBD_KEY_CODES] = { 0, 0, 0, 0, 0, 0 };
                    if (sc->multitaskingy < 0)
                        keyCodes[0] = 0x52;
                    else
                        keyCodes[0] = 0x51;
                    update_keyboard(shiftKeys, keyCodes);
                    shiftKeys = 0;
                    keyCodes[0] = 0x0;
                    update_keyboard(shiftKeys, keyCodes);
                    sc->multitaskingx = 0;
                    sc->multitaskingy = 0;
                    sc->multitaskingdone = true;
                }
            }
            else {
                if (abs(sc->multitaskingx) > 50) {
                    uint8_t shiftKeys = KBD_LCONTROL_BIT;
                    uint8_t keyCodes[KBD_KEY_CODES] = { 0, 0, 0, 0, 0, 0 };
                    if (sc->multitaskingx > 0)
                        keyCodes[0] = 0x50;
                    else
                        keyCodes[0] = 0x4F;
                    update_keyboard(shiftKeys, keyCodes);
                    shiftKeys = 0;
                    keyCodes[0] = 0x0;
                    update_keyboard(shiftKeys, keyCodes);
                    sc->multitaskingx = 0;
                    sc->multitaskingy = 0;
                    sc->multitaskingdone = true;
                }
            }
        }
        else if (sc->multitaskinggesturetick > 25) {
            sc->multitaskingx = 0;
            sc->multitaskingy = 0;
            sc->multitaskinggesturetick = 0;
            sc->multitaskingdone = false;
        }
        return true;
    }
    else {
        sc->multitaskingx = 0;
        sc->multitaskingy = 0;
        sc->multitaskinggesturetick = 0;
        sc->multitaskingdone = false;
        return false;
    }
}

void VoodooI2CElanTouchpadDevice::TapToClickOrDrag(csgesture_softc *sc, int button) {
    sc->tickssinceclick++;
    if (sc->mouseDownDueToTap && sc->idForMouseDown == -1) {
        if (sc->tickssinceclick > 10) {
            sc->mouseDownDueToTap = false;
            sc->mousedown = false;
            sc->buttonmask = 0;
            //Tap Drag Timed out
        }
        return;
    }
    if (sc->mousedown) {
        sc->tickssinceclick = 0;
        return;
    }
    if (button == 0)
        return;
    
    for (int i = 0; i < MAX_FINGERS; i++){
        if (sc->truetick[i] < 10 && sc->truetick[i] > 0)
            button++;
    }
    
    int buttonmask = 0;
    
    switch (button) {
        case 1:
            buttonmask = MOUSE_BUTTON_1;
            break;
        case 2:
            buttonmask = MOUSE_BUTTON_2;
            break;
        case 3:
            buttonmask = MOUSE_BUTTON_3;
            break;
    }
    if (buttonmask != 0 && sc->tickssinceclick > 10 && sc->ticksincelastrelease == 0) {
        sc->idForMouseDown = -1;
        sc->mouseDownDueToTap = true;
        sc->buttonmask = buttonmask;
        sc->mousebutton = button;
        sc->mousedown = true;
        sc->tickssinceclick = 0;
    }
}

void VoodooI2CElanTouchpadDevice::ClearTapDrag(csgesture_softc *sc, int i) {
    if (i == sc->idForMouseDown && sc->mouseDownDueToTap == true) {
        if (sc->tick[i] < 10) {
            //Double Tap
            update_relative_mouse(0, 0, 0, 0, 0);
            update_relative_mouse(sc->buttonmask, 0, 0, 0, 0);
        }
        sc->mouseDownDueToTap = false;
        sc->mousedown = false;
        sc->buttonmask = 0;
        sc->idForMouseDown = -1;
        //Clear Tap Drag
    }
}

void VoodooI2CElanTouchpadDevice::ProcessGesture(csgesture_softc *sc) {
#pragma mark reset inputs
    sc->dx = 0;
    sc->dy = 0;
    
#pragma mark process touch thresholds
    int avgx[MAX_FINGERS];
    int avgy[MAX_FINGERS];
    
    int abovethreshold = 0;
    int recentlyadded = 0;
    int iToUse[3] = { -1,-1,-1 };
    int a = 0;
    
    int nfingers = 0;
    for (int i = 0;i < MAX_FINGERS;i++) {
        if (sc->x[i] != -1)
            nfingers++;
    }
    
    for (int i = 0;i < MAX_FINGERS;i++) {
        if (sc->truetick[i] < 30 && sc->truetick[i] != 0)
            recentlyadded++;
        if (sc->tick[i] == 0)
            continue;
        if (sc->blacklistedids[i] == 1)
            continue;
        avgx[i] = sc->flextotalx[i] / sc->tick[i];
        avgy[i] = sc->flextotaly[i] / sc->tick[i];
        if (distancesq(avgx[i], avgy[i]) > 2) {
            abovethreshold++;
            iToUse[a] = i;
            a++;
        }
    }
    
#pragma mark process different gestures
    bool handled = false;
    if (!handled)
        handled = ProcessThreeFingerSwipe(sc, abovethreshold, iToUse);
    if (!handled)
        handled = ProcessScroll(sc, abovethreshold, iToUse);
    if (!handled)
        handled = ProcessMove(sc, abovethreshold, iToUse);
    
#pragma mark process clickpad press state
    int buttonmask = 0;
    
    sc->mousebutton = recentlyadded;
    if (sc->mousebutton == 0)
        sc->mousebutton = abovethreshold;
    
    if (sc->mousebutton == 0) {
        if (sc->panningActive)
            sc->mousebutton = 1;
        else
            sc->mousebutton = nfingers;
        if (sc->mousebutton == 0)
            sc->mousebutton = 1;
    }
    if (sc->mousebutton > 3)
        sc->mousebutton = 3;
    
    if (!sc->mouseDownDueToTap) {
        if (sc->buttondown && !sc->mousedown) {
            sc->mousedown = true;
            sc->tickssinceclick = 0;
            
            switch (sc->mousebutton) {
                case 1:
                    buttonmask = MOUSE_BUTTON_1;
                    break;
                case 2:
                    buttonmask = MOUSE_BUTTON_2;
                    break;
                case 3:
                    buttonmask = MOUSE_BUTTON_3;
                    break;
            }
            sc->buttonmask = buttonmask;
        }
        else if (sc->mousedown && !sc->buttondown) {
            sc->mousedown = false;
            sc->mousebutton = 0;
            sc->buttonmask = 0;
        }
    }
    
#pragma mark shift to last
    int releasedfingers = 0;
    
    for (int i = 0;i < MAX_FINGERS;i++) {
        if (sc->x[i] != -1) {
            if (sc->lastx[i] == -1) {
                if (sc->ticksincelastrelease < 10 && sc->mouseDownDueToTap && sc->idForMouseDown == -1) {
                    sc->idForMouseDown = i; //Associate Tap Drag
                }
            }
            sc->truetick[i]++;
            if (sc->tick[i] < 10) {
                if (sc->lastx[i] != -1) {
                    sc->totalx[i] += abs(sc->x[i] - sc->lastx[i]);
                    sc->totaly[i] += abs(sc->y[i] - sc->lasty[i]);
                    sc->totalp[i] += sc->p[i];
                    
                    sc->flextotalx[i] = sc->totalx[i];
                    sc->flextotaly[i] = sc->totaly[i];
                    
                    int j = sc->tick[i];
                    sc->xhistory[i][j] = abs(sc->x[i] - sc->lastx[i]);
                    sc->yhistory[i][j] = abs(sc->y[i] - sc->lasty[i]);
                }
                sc->tick[i]++;
            }
            else if (sc->lastx[i] != -1) {
                int absx = abs(sc->x[i] - sc->lastx[i]);
                int absy = abs(sc->y[i] - sc->lasty[i]);
                
                int newtotalx = sc->flextotalx[i] - sc->xhistory[i][0] + absx;
                int newtotaly = sc->flextotaly[i] - sc->yhistory[i][0] + absy;
                
                sc->totalx[i] += absx;
                sc->totaly[i] += absy;
                
                sc->flextotalx[i] -= sc->xhistory[i][0];
                sc->flextotaly[i] -= sc->yhistory[i][0];
                for (int j = 1;j < 10;j++) {
                    sc->xhistory[i][j - 1] = sc->xhistory[i][j];
                    sc->yhistory[i][j - 1] = sc->yhistory[i][j];
                }
                sc->flextotalx[i] += absx;
                sc->flextotaly[i] += absy;
                
                int j = 9;
                sc->xhistory[i][j] = absx;
                sc->yhistory[i][j] = absy;
            }
        }
        if (sc->x[i] == -1) {
            ClearTapDrag(sc, i);
            if (sc->lastx[i] != -1)
                sc->ticksincelastrelease = -1;
            for (int j = 0;j < 10;j++) {
                sc->xhistory[i][j] = 0;
                sc->yhistory[i][j] = 0;
            }
            if (sc->tick[i] < 10 && sc->tick[i] != 0) {
                releasedfingers++;
            }
            sc->totalx[i] = 0;
            sc->totaly[i] = 0;
            sc->totalp[i] = 0;
            sc->tick[i] = 0;
            sc->truetick[i] = 0;
            
            sc->blacklistedids[i] = 0;
            
            if (sc->idForPanning == i) {
                sc->panningActive = false;
                sc->idForPanning = -1;
            }
        }
        sc->lastx[i] = sc->x[i];
        sc->lasty[i] = sc->y[i];
        sc->lastp[i] = sc->p[i];
    }
    sc->ticksincelastrelease++;
    
#pragma mark process tap to click
    TapToClickOrDrag(sc, releasedfingers);
    
#pragma mark send to system
    update_relative_mouse(sc->buttonmask, sc->dx, sc->dy, sc->scrolly, sc->scrollx);
}

void VoodooI2CElanTouchpadDevice::TrackpadRawInput(struct csgesture_softc *sc, uint8_t report[ETP_MAX_REPORT_LEN], int tickinc){
    if (report[0] == 0xff) {
        return;
    }
    
    uint8_t *finger_data = &report[ETP_FINGER_DATA_OFFSET];
    int i;
    uint8_t tp_info = report[ETP_TOUCH_INFO_OFFSET];
    uint8_t hover_info = report[ETP_HOVER_INFO_OFFSET];
    bool contact_valid, hover_event;
    
    int nfingers = 0;
    
    for (int i = 0;i < 5; i++) {
        sc->x[i] = -1;
        sc->y[i] = -1;
        sc->p[i] = -1;
    }
    
    hover_event = hover_info & 0x40;
    for (i = 0; i < ETP_MAX_FINGERS; i++) {
        contact_valid = tp_info & (1U << (3 + i));
        unsigned int pos_x, pos_y;
        unsigned int pressure, mk_x, mk_y;
        unsigned int area_x, area_y, major, minor;
        unsigned int scaled_pressure;
        
        if (contact_valid) {
            pos_x = ((finger_data[0] & 0xf0) << 4) |
            finger_data[1];
            pos_y = ((finger_data[0] & 0x0f) << 8) |
            finger_data[2];
            
            mk_x = (finger_data[3] & 0x0f);
            mk_y = (finger_data[3] >> 4);
            pressure = finger_data[4];
            
            //map to cypress coordinates
            //pos_y = 1500 - pos_y;
            pos_y = sc->phyy - pos_y;
            pos_x *= 2;
            pos_x /= 7;
            pos_y *= 2;
            pos_y /= 7;
            
            
            /*
             * To avoid treating large finger as palm, let's reduce the
             * width x and y per trace.
             */
            area_x = mk_x;
            area_y = mk_y;
            
            major = max(area_x, area_y);
            minor = min(area_x, area_y);
            
            scaled_pressure = pressure;
            
            if (scaled_pressure > ETP_MAX_PRESSURE)
                scaled_pressure = ETP_MAX_PRESSURE;
            sc->x[i] = pos_x;
            sc->y[i] = pos_y;
            sc->p[i] = scaled_pressure;
        }
        else {
        }
        
        if (contact_valid) {
            finger_data += ETP_FINGER_DATA_LEN;
            nfingers++;
        }
    }
    sc->buttondown = (tp_info & 0x01);
    
    ProcessGesture(sc);
}

bool VoodooI2CElanTouchpadDevice::attach(IOService * provider, IOService* child)
{
    if (!super::attach(provider))
        return false;
    
    assert(_controller == 0);
    _controller = (VoodooI2C*)provider;
    _controller->retain();
    
    
    child->attach(this);
    if (!probe(child))
        return false;
    
    return true;
}

bool VoodooI2CElanTouchpadDevice::probe(IOService* device) {
    
    
    hid_device = (I2CDevice *)IOMalloc(sizeof(I2CDevice));
    
    //hid_device->_dev = _controller->_dev;
    
    if (!super::start(device))
        return false;
    
    
    hid_device->provider = OSDynamicCast(IOACPIPlatformDevice, device);
    hid_device->provider->retain();
    
    int ret = i2c_get_slave_address(hid_device);
    if (ret < 0){
        IOLog("%s::%s::Failed to get a slave address for an I2C device, aborting.\n", getName(), _controller->_dev->name);
        IOFree(hid_device, sizeof(I2CDevice));
        return false;
    }
    
    
    IOLog("%s::%s::HID Probe called for i2c 0x%02x\n", getName(), _controller->_dev->name, hid_device->addr);
    
    initHIDDevice(hid_device);
    
    //super::stop(device);
    return 0;
}

void VoodooI2CElanTouchpadDevice::stop(IOService* device) {
    
    IOLog("I2C HID Device is stopping\n");
    
    destroy_wrapper();
    
    if (hid_device->timerSource){
        hid_device->timerSource->cancelTimeout();
        hid_device->timerSource->release();
        hid_device->timerSource = NULL;
    }
    
    //hid_device->workLoop->removeEventSource(hid_device->interruptSource);
    //hid_device->interruptSource->disable();
    hid_device->interruptSource = NULL;
    
    hid_device->workLoop->release();
    hid_device->workLoop = NULL;
    
    IOFree(hid_device, sizeof(I2CDevice));
    
    //hid_device->provider->close(this);
    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void VoodooI2CElanTouchpadDevice::detach( IOService * provider )
{
    assert(_controller == provider);
    _controller->release();
    _controller = 0;
    
    super::detach(provider);
}

int VoodooI2CElanTouchpadDevice::initHIDDevice(I2CDevice *hid_device) {
    PMinit();
    
    int ret;
    UInt16 hidRegister;
    
    elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_RESET);
    
    uint8_t val[256];
    readI2C(0x00, ETP_I2C_INF_LENGTH, val);
    
    readI2C16(ETP_I2C_DESC_CMD, ETP_I2C_DESC_LENGTH, val);
    
    readI2C16(ETP_I2C_REPORT_DESC_CMD, ETP_I2C_REPORT_DESC_LENGTH, val);
    
    elan_i2c_write_cmd(ETP_I2C_SET_CMD, ETP_ENABLE_ABS);
    
    elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_WAKE_UP);
    
    uint8_t val2[3];
    
    elan_i2c_read_cmd(ETP_I2C_UNIQUEID_CMD, val2);
    uint8_t prodid = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_FW_VERSION_CMD, val2);
    uint8_t version = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_FW_CHECKSUM_CMD, val2);
    uint16_t csum = *((uint16_t *)val2);
    
    elan_i2c_read_cmd(ETP_I2C_SM_VERSION_CMD, val2);
    uint8_t smvers = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_IAP_VERSION_CMD, val2);
    uint8_t iapversion = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_PRESSURE_CMD, val2);
    
    elan_i2c_read_cmd(ETP_I2C_MAX_X_AXIS_CMD, val2);
    uint16_t max_x = (*((uint16_t *)val2)) & 0x0fff;
    
    elan_i2c_read_cmd(ETP_I2C_MAX_Y_AXIS_CMD, val2);
    uint16_t max_y = (*((uint16_t *)val2)) & 0x0fff;
    
    elan_i2c_read_cmd(ETP_I2C_XY_TRACENUM_CMD, val2);
    
    uint8_t x_traces = val2[0];
    uint8_t y_traces = val2[1];
    
    csgesture_softc *sc = &softc;
    
    sprintf(sc->product_id, "%d.0", prodid);
    sprintf(sc->firmware_version, "%d.0", version);
    
    sc->resx = max_x;
    sc->resy = max_y;
    
    sc->resx *= 2;
    sc->resx /= 7;
    
    sc->resy *= 2;
    sc->resy /= 7;
    
    sc->phyx = max_x;
    sc->phyy = max_y;
    
    IOLog("%s::%s::[Elan Trackpad] ProdID: %d Vers: %d Csum: %d SmVers: %d IAPVers: %d Max X: %d Max Y: %d\n", getName(), _controller->_dev->name, prodid, version, csum, smvers, iapversion, max_x, max_y);
    
    elan_i2c_write_cmd(ETP_I2C_SET_CMD, ETP_ENABLE_CALIBRATE | ETP_ENABLE_ABS);
    
    elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_WAKE_UP);
    
    elan_i2c_write_cmd(ETP_I2C_CALIBRATE_CMD, 1);
    
    readI2C16(ETP_I2C_CALIBRATE_CMD, 1, val2);
    
    elan_i2c_write_cmd(ETP_I2C_SET_CMD, ETP_ENABLE_ABS);
    
    sc->infoSetup = true;
    
    hid_device->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!hid_device->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _controller->_dev->name);
        stop(this);
        return -1;
    }
    
    hid_device->workLoop->retain();
    
    /*
     hid_device->interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CHIDDevice::InterruptOccured), hid_device->provider);
     
     if (hid_device->workLoop->addEventSource(hid_device->interruptSource) != kIOReturnSuccess) {
     IOLog("%s::%s::Could not add interrupt source to workloop\n", getName(), _controller->_dev->name);
     stop(this);
     return -1;
     }
     
     hid_device->interruptSource->enable();
     */
    
    hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CElanTouchpadDevice::get_input));
    if (!hid_device->timerSource){
        IOLog("%s", "Timer Err!\n");
        goto err;
    }
    
    hid_device->workLoop->addEventSource(hid_device->timerSource);
    hid_device->timerSource->setTimeoutMS(10);
    /*
     
     hid_device->commandGate = IOCommandGate::commandGate(this);
     
     if (!hid_device->commandGate || (_dev->workLoop->addEventSource(hid_device->commandGate) != kIOReturnSuccess)) {
     IOLog("%s::%s::Failed to open HID command gate\n", getName(), _dev->name);
     return -1;
     }
     */
    
    initialize_wrapper();
    registerService();
    
#define kMyNumberOfStates 2
    
    static IOPMPowerState myPowerStates[kMyNumberOfStates];
    // Zero-fill the structures.
    bzero (myPowerStates, sizeof(myPowerStates));
    // Fill in the information about your device's off state:
    myPowerStates[0].version = 1;
    myPowerStates[0].capabilityFlags = kIOPMPowerOff;
    myPowerStates[0].outputPowerCharacter = kIOPMPowerOff;
    myPowerStates[0].inputPowerRequirement = kIOPMPowerOff;
    // Fill in the information about your device's on state:
    myPowerStates[1].version = 1;
    myPowerStates[1].capabilityFlags = kIOPMPowerOn;
    myPowerStates[1].outputPowerCharacter = kIOPMPowerOn;
    myPowerStates[1].inputPowerRequirement = kIOPMPowerOn;
    
    
    
    this->_controller->joinPMtree(this);
    
    registerPowerDriver(this, myPowerStates, kMyNumberOfStates);
    
    return 0;
    
err:
    return ret;
}

void VoodooI2CElanTouchpadDevice::initialize_wrapper(void) {
    destroy_wrapper();
    
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    _wrapper = new VoodooElanTouchpadWrapper;
    if (_wrapper->init()) {
        IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
        _wrapper->attach(this);
        _wrapper->start(this);
    }
    else {
        IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
        _wrapper->release();
        _wrapper = NULL;
    }
}

void VoodooI2CElanTouchpadDevice::destroy_wrapper(void) {
    if (_wrapper != NULL) {
        _wrapper->terminate(kIOServiceRequired | kIOServiceSynchronous);
        _wrapper->release();
        _wrapper = NULL;
    }
}

#define EIO              5      /* I/O error */
#define ENOMEM          12      /* Out of memory */

void VoodooI2CElanTouchpadDevice::elan_i2c_read_cmd(uint16_t reg, uint8_t *val) {
    readI2C16(reg, ETP_I2C_INF_LENGTH, val);
}

void VoodooI2CElanTouchpadDevice::elan_i2c_write_cmd(uint16_t reg, uint16_t cmd){
    uint16_t buffer[] = { cmd };
    readI2C16(reg, sizeof(buffer), (uint8_t *)buffer);
}

SInt32 VoodooI2CElanTouchpadDevice::readI2C(uint8_t reg, size_t len, uint8_t *values){
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = 1,
            .buf = &reg,
        },
        {
            .addr = hid_device->addr,
            .flags = I2C_M_RD,
            .len = (uint8_t)len,
            .buf = values,
        },
    };
    int ret;
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)msgs, ARRAY_SIZE(msgs));
    if (ret != ARRAY_SIZE(msgs))
        return ret < 0 ? ret : EIO;
    return 0;
}

SInt32 VoodooI2CElanTouchpadDevice::readI2C16(uint16_t reg, size_t len, uint8_t *values){
    uint16_t buf[] {
        reg
    };
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = sizeof(buf),
            .buf = (uint8_t *)&buf,
        },
        {
            .addr = 0x67,
            .flags = I2C_M_RD,
            .len = (uint8_t)len,
            .buf = values,
        },
    };
    int ret;
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)msgs, ARRAY_SIZE(msgs));
    if (ret != ARRAY_SIZE(msgs))
        return ret < 0 ? ret : EIO;
    return 0;
}

SInt32 VoodooI2CElanTouchpadDevice::writeI2C(uint8_t reg, size_t len, uint8_t *values){
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = 1,
            .buf = &reg,
        },
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = (uint8_t)len,
            .buf = values,
        },
    };
    int ret;
    
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)&msgs, ARRAY_SIZE(msgs));
    
    return ret;
}

SInt32 VoodooI2CElanTouchpadDevice::writeI2C16(uint16_t reg, size_t len, uint8_t *values){
    uint16_t buf[] {
        reg
    };
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = sizeof(buf),
            .buf = (uint8_t *)&buf,
        },
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = (uint8_t)len,
            .buf = values,
        },
    };
    int ret;
    
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)&msgs, ARRAY_SIZE(msgs));
    
    return ret;
}

IOReturn VoodooI2CElanTouchpadDevice::setPowerState(unsigned long powerState, IOService *whatDevice){
    if (powerState == 0){
        //Going to sleep
        if (hid_device->timerSource){
            hid_device->timerSource->cancelTimeout();
            hid_device->timerSource->release();
            hid_device->timerSource = NULL;
        }
        IOLog("%s::Going to Sleep!\n", getName());
    } else {
        //Waking up from Sleep
        if (!hid_device->timerSource){
            hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CElanTouchpadDevice::get_input));
            hid_device->workLoop->addEventSource(hid_device->timerSource);
            hid_device->timerSource->setTimeoutMS(10);
        }
        
        IOLog("%s::Woke up from Sleep!\n", getName());
    }
    return kIOPMAckImplied;
}

int VoodooI2CElanTouchpadDevice::i2c_get_slave_address(I2CDevice* hid_device){
    OSObject* result = NULL;
    
    hid_device->provider->evaluateObject("_CRS", &result);
    
    OSData* data = OSDynamicCast(OSData, result);
    
    hid_device->addr = *(int*)data->getBytesNoCopy(16,1) & 0xFF;
    
    data->release();
    
    return 0;
    
}

void VoodooI2CElanTouchpadDevice::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    IOLog("interrupt\n");
    if (hid_device->reading)
        return;
    //i2c_hid_get_input(ihid);
}

void VoodooI2CElanTouchpadDevice::get_input(OSObject* owner, IOTimerEventSource* sender) {
    uint8_t report[ETP_MAX_REPORT_LEN];
    readI2C(0, sizeof(report), report);
    
    TrackpadRawInput(&softc, report, 1);
    hid_device->timerSource->setTimeoutMS(10);
}

static _ELAN_RELATIVE_MOUSE_REPORT lastreport;

void VoodooI2CElanTouchpadDevice::update_relative_mouse(char button,
                                                     char x, char y, char wheelPosition, char wheelHPosition){
    _ELAN_RELATIVE_MOUSE_REPORT report;
    report.ReportID = REPORTID_RELATIVE_MOUSE;
    report.Button = button;
    report.XValue = x;
    report.YValue = y;
    report.WheelPosition = wheelPosition;
    report.HWheelPosition = wheelHPosition;
    
    if (report.Button == lastreport.Button &&
        report.XValue == lastreport.XValue &&
        report.YValue == lastreport.YValue &&
        report.WheelPosition == lastreport.WheelPosition &&
        report.HWheelPosition == lastreport.HWheelPosition)
        return;
    lastreport = report;
    
    lastmouse.x = x;
    lastmouse.y = y;
    lastmouse.buttonMask = button;
    
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, sizeof(report));
    buffer->writeBytes(0, &report, sizeof(report));
    
    IOReturn err = _wrapper->handleReport(buffer, kIOHIDReportTypeInput);
    if (err != kIOReturnSuccess)
        IOLog("Error handling report: 0x%.8x\n", err);
    
    buffer->release();
}

void VoodooI2CElanTouchpadDevice::update_keyboard(uint8_t shiftKeys, uint8_t keyCodes[KBD_KEY_CODES]) {
    _ELAN_KEYBOARD_REPORT report;
    report.ReportID = REPORTID_KEYBOARD;
    report.ShiftKeyFlags = shiftKeys;
    for (int i = 0; i < KBD_KEY_CODES; i++){
        report.KeyCodes[i] = keyCodes[i];
    }
    
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, sizeof(report));
    buffer->writeBytes(0, &report, sizeof(report));
    
    IOReturn err = _wrapper->handleReport(buffer, kIOHIDReportTypeInput);
    if (err != kIOReturnSuccess)
        IOLog("Error handling report: 0x%.8x\n", err);
    
    buffer->release();

}

int VoodooI2CElanTouchpadDevice::reportDescriptorLength(){
    return sizeof(elandesc);
}

int VoodooI2CElanTouchpadDevice::vendorID(){
    return 'pyaC';
}

int VoodooI2CElanTouchpadDevice::productID(){
    return 'rtyC';
}

void VoodooI2CElanTouchpadDevice::write_report_to_buffer(IOMemoryDescriptor *buffer){
    
    _ELAN_RELATIVE_MOUSE_REPORT report;
    report.ReportID = REPORTID_RELATIVE_MOUSE;
    report.Button = lastmouse.buttonMask;
    report.XValue = lastmouse.x;
    report.YValue = lastmouse.y;
    report.WheelPosition = 0;
    report.HWheelPosition = 0;
    
    UInt rsize = sizeof(report);
    
    buffer->writeBytes(0, &report, rsize);
}

void VoodooI2CElanTouchpadDevice::write_report_descriptor_to_buffer(IOMemoryDescriptor *buffer){
    
    UInt rsize = sizeof(elandesc);
    
    buffer->writeBytes(0, elandesc, rsize);
}
