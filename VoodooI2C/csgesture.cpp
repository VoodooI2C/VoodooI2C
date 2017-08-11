// CSGesture Multitouch Touchpad Library
// © 2016, CoolStar. All Rights Reserved.

#include "csgesture.h"

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

#ifndef ABS32
#define ABS32
inline int32_t abs(int32_t num){
    if (num < 0){
        return num * -1;
    }
    return num;
}
#endif

inline int filterNegative(int val) {
    if (val > 0)
        return val;
    return 65535;
}

typedef unsigned char BYTE;

unsigned char csgesturedesc[] = {
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

typedef struct __attribute__((__packed__)) _CSGESTURE_KEYBOARD_REPORT
{
    
    BYTE      ReportID;
    
    // Left Control, Left Shift, Left Alt, Left GUI
    // Right Control, Right Shift, Right Alt, Right GUI
    BYTE      ShiftKeyFlags;
    
    BYTE      Reserved;
    
    // See http://www.usb.org/developers/devclass_docs/Hut1_11.pdf
    // for a list of key codes
    BYTE      KeyCodes[KBD_KEY_CODES];
    
} CSGestureKeyboardReport;

int CSGesture::distancesq(int delta_x, int delta_y){
    return (delta_x * delta_x) + (delta_y*delta_y);
}

bool CSGesture::ProcessMove(csgesture_softc *sc, int abovethreshold, int iToUse[3]) {
    int frequmult = 10 / sc->frequency;
    
    if (abovethreshold == 1 || sc->panningActive) {
        int i = iToUse[0];
        if (!sc->panningActive && sc->tick[i] < (5 * frequmult))
            return false;
        
        if (_scrollHandler){
            _scrollHandler->softc = sc;
            _scrollHandler->stopScroll();
        }
        
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
                        if (sc->truetick[j] > sc->truetick[i] + (15 * frequmult)) {
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

bool CSGesture::ProcessScroll(csgesture_softc *sc, int abovethreshold, int iToUse[3]) {
    int frequmult = 10 / sc->frequency;
    
    sc->scrollx = 0;
    sc->scrolly = 0;
    
    if (abovethreshold == 2 || sc->scrollingActive) {
        int i1 = iToUse[0];
        int i2 = iToUse[1];
        
        if (!sc->scrollingActive && !sc->scrollInertiaActive) {
            if (sc->truetick[i1] < (8 * frequmult) && sc->truetick[i2] < (8 * frequmult))
                return false;
        }
        
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
        
#if 0
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
#endif
        
        int scrollx = 0;
        int scrolly = 0;
        
        if ((abs(delta_y1) + abs(delta_y2)) > (abs(delta_x1) + abs(delta_x2))) {
            int avgy = (delta_y1 + delta_y2) / 2;
            scrolly = avgy;
        }
        else {
            int avgx = (delta_x1 + delta_x2) / 2;
            scrollx = avgx;
        }
        
        if (abs(scrollx) < 5 && abs(scrolly) < 5 && !sc->scrollingActive)
            return false;

        if (_scrollHandler){
            _scrollHandler->softc = sc;
            _scrollHandler->ProcessScroll(filterNegative(sc->x[i1]),
                                      filterNegative(sc->y[i1]),
                                      filterNegative(sc->x[i2]),
                                      filterNegative(sc->y[i2]));
        }
        //_pointingWrapper->updateScroll(sc->scrolly, sc->scrollx, 0);
        
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
        if (fngrcount == 2 || sc->ticksSinceScrolling <= (5 * frequmult)) {
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

bool CSGesture::ProcessThreeFingerSwipe(csgesture_softc *sc, int abovethreshold, int iToUse[3]) {
    if (abovethreshold == 3 || abovethreshold == 4) {
        if (_scrollHandler){
            _scrollHandler->softc = sc;
            _scrollHandler->stopScroll();
        }
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

void CSGesture::TapToClickOrDrag(csgesture_softc *sc, int button) {
    int freqmult = 10 / sc->frequency;
    if (!sc->settings.tapToClickEnabled)
        return;
    
    sc->tickssinceclick++;
    if (sc->mouseDownDueToTap && sc->idForMouseDown == -1) {
        if (sc->tickssinceclick > (10 * freqmult)) {
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
    
    for (int i = 0; i < MAX_FINGERS; i++){
        if (sc->truetick[i] < 10 && sc->truetick[i] > 0)
            button++;
    }
    
    if (button == 0)
        return;
    
    if (_scrollHandler){
        if (_scrollHandler->isScrolling()){
            _scrollHandler->stopScroll();
            return;
        }
    }
    
    int buttonmask = 0;
    
    switch (button) {
        case 1:
            buttonmask = MOUSE_BUTTON_1;
            break;
        case 2:
            if (sc->settings.multiFingerTap)
                buttonmask = MOUSE_BUTTON_2;
            break;
        case 3:
            if (sc->settings.multiFingerTap)
                buttonmask = MOUSE_BUTTON_3;
            break;
    }
    if (buttonmask != 0 && sc->tickssinceclick > (10 * freqmult) && sc->ticksincelastrelease == 0) {
        sc->idForMouseDown = -1;
        sc->mouseDownDueToTap = true;
        sc->buttonmask = buttonmask;
        sc->mousebutton = button;
        sc->mousedown = true;
        sc->tickssinceclick = 0;
    }
}

void CSGesture::ClearTapDrag(csgesture_softc *sc, int i) {
    int frequmult = 10 / sc->frequency;
    if (i == sc->idForMouseDown && sc->mouseDownDueToTap == true) {
        if (sc->tick[i] < (10 * frequmult)) {
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

void CSGesture::ProcessGesture(csgesture_softc *sc) {
    int frequmult = 10 / sc->frequency;
#pragma mark reset inputs
    sc->dx = 0;
    sc->dy = 0;
    
#pragma mark process touch thresholds
    int avgx[MAX_FINGERS];
    int avgy[MAX_FINGERS];
    
    int abovethreshold = 0;
    int recentlyadded = 0;
    int iToUse[5] = { -1,-1,-1, -1, -1};
    int a = 0;
    
    int nfingers = 0;
    for (int i = 0;i < MAX_FINGERS;i++) {
        if (sc->x[i] != -1)
            nfingers++;
    }
    
    for (int i = 0;i < MAX_FINGERS;i++) {
        if (sc->truetick[i] < (30 * frequmult) && sc->truetick[i] != 0)
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
    bool handledByScroll = false;
    
    if (!handled)
        handled = ProcessThreeFingerSwipe(sc, abovethreshold, iToUse);
    if (!handled && !sc->buttondown && !sc->mouseDownDueToTap)
        handledByScroll = handled = ProcessScroll(sc, abovethreshold, iToUse);
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
            if (_scrollHandler){
                if (_scrollHandler->isScrolling()){
                    _scrollHandler->stopScroll();
                }
            }
            
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
                if (sc->ticksincelastrelease < (10 * frequmult) && sc->mouseDownDueToTap && sc->idForMouseDown == -1) {
                    if (sc->settings.tapDragEnabled)
                        sc->idForMouseDown = i; //Associate Tap Drag
                }
            }
            sc->truetick[i]++;
            if (sc->tick[i] < (10 * frequmult)) {
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
                
                sc->totalx[i] += absx;
                sc->totaly[i] += absy;
                
                sc->flextotalx[i] -= sc->xhistory[i][0];
                sc->flextotaly[i] -= sc->yhistory[i][0];
                for (int j = 1;j < (10 * frequmult);j++) {
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
            for (int j = 0;j < (10 * frequmult);j++) {
                sc->xhistory[i][j] = 0;
                sc->yhistory[i][j] = 0;
            }
            if (sc->tick[i] < (10 * frequmult) && sc->tick[i] != 0) {
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
    if (!handledByScroll)
        TapToClickOrDrag(sc, releasedfingers);
    
#pragma mark send to system
    update_relative_mouse(sc->buttonmask, sc->dx, sc->dy, sc->scrolly, sc->scrollx);
}

#pragma mark OS Specific functions

void CSGesture::prepareToSleep(){
    if (_scrollHandler)
        _scrollHandler->prepareToSleep();
}

void CSGesture::wakeFromSleep(){
    if (_scrollHandler)
        _scrollHandler->wakeFromSleep();
}

void CSGesture::initialize_wrapper(IOService *service) {
    _wrapper = NULL;
    _pointingWrapper = NULL;
    _scrollHandler = NULL;
    
    _wrapper = new VoodooCSGestureHIDWrapper;
    if (_wrapper->init()) {
        _wrapper->gestureEngine = this;
        _wrapper->attach(service);
        _wrapper->start(service);
    }
    else {
        _wrapper->release();
        _wrapper = NULL;
    }
    
    _pointingWrapper = new VoodooCSGestureHIPointingWrapper;
    if (_pointingWrapper->init()){
        _pointingWrapper->gesturerec = this;
        _pointingWrapper->attach(service);
        _pointingWrapper->start(service);
    } else {
        _pointingWrapper->release();
        _pointingWrapper = NULL;
    }
    
    _scrollHandler = new CSGestureScroll;
    if (_scrollHandler->init()){
        _scrollHandler->softc = softc;
        _scrollHandler->inertiaScroll = true;
        
        _scrollHandler->_pointingWrapper = _pointingWrapper;
        _scrollHandler->_gestureEngine = this;
        
        _scrollHandler->attach(service);
        _scrollHandler->start(service);
    }
}

void CSGesture::destroy_wrapper(void) {
    if (_scrollHandler != NULL){
        _scrollHandler->stop();
        _scrollHandler->_pointingWrapper = NULL;
        _scrollHandler->_gestureEngine = NULL;
        _scrollHandler->terminate(kIOServiceRequired | kIOServiceSynchronous);
        _scrollHandler->release();
        _scrollHandler = NULL;
    }
    
    if (_wrapper != NULL) {
        _wrapper->terminate(kIOServiceRequired | kIOServiceSynchronous);
        _wrapper->release();
        _wrapper = NULL;
    }
    
    if (_pointingWrapper != NULL){
        _pointingWrapper->terminate(kIOServiceRequired | kIOServiceSynchronous);
        _pointingWrapper->release();
        _pointingWrapper = NULL;
    }
}

void CSGesture::update_relative_mouse(char button, char x, char y, char wheelPosition, char wheelHPosition){
    if (_pointingWrapper)
        _pointingWrapper->updateRelativeMouse(x, y, button);
}

void CSGesture::update_keyboard(uint8_t shiftKeys, uint8_t keyCodes[KBD_KEY_CODES]) {
    _CSGESTURE_KEYBOARD_REPORT report;
    report.ReportID = REPORTID_KEYBOARD;
    report.ShiftKeyFlags = shiftKeys;
    for (int i = 0; i < KBD_KEY_CODES; i++){
        report.KeyCodes[i] = keyCodes[i];
    }
    
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, sizeof(report));
    buffer->writeBytes(0, &report, sizeof(report));
    
    if (_wrapper){
        IOReturn err = _wrapper->handleReport(buffer, kIOHIDReportTypeInput);
        if (err != kIOReturnSuccess)
            IOLog("Error handling report: 0x%.8x\n", err);
    }
    buffer->release();
    
}

int CSGesture::reportDescriptorLength(){
    return sizeof(csgesturedesc);
}

void CSGesture::write_report_to_buffer(IOMemoryDescriptor *buffer){
    
    _CSGESTURE_KEYBOARD_REPORT report;
    report.ReportID = REPORTID_KEYBOARD;
    report.ShiftKeyFlags = 0;
    for (int i = 0; i < KBD_KEY_CODES; i++){
        report.KeyCodes[i] = 0;
    }
    
    UInt rsize = sizeof(report);
    
    buffer->writeBytes(0, &report, rsize);
}

void CSGesture::write_report_descriptor_to_buffer(IOMemoryDescriptor *buffer){
    
    UInt rsize = sizeof(csgesturedesc);
    
    buffer->writeBytes(0, csgesturedesc, rsize);
}
