// CSGesture Multitouch Touchpad Library
// © 2016, CoolStar. All Rights Reserved.

#ifndef csgesturestruct_h
#define csgesturestruct_h

#include <stdint.h>
#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include "VoodooCSGestureHIDWrapper.h"
#include "VoodooCSGestureHIPointingWrapper.h"
#include "csgesture-softc.h"
#include "csgesturescroll.h"

#define MAX_FINGERS 15

class CSGesture {
private:
    VoodooCSGestureHIDWrapper *_wrapper;
    VoodooCSGestureHIPointingWrapper *_pointingWrapper;
    CSGestureScroll *_scrollHandler;
    
    struct {
        UInt8 x;
        UInt8 y;
        UInt8 buttonMask;
    } lastmouse;
    
    int distancesq(int delta_x, int delta_y);
    
    //os callbacks
    void update_relative_mouse(char button,
                               char x, char y, char wheelPosition, char wheelHPosition);
    void update_keyboard(uint8_t shiftKeys, uint8_t *keyCodes);
public:
    csgesture_softc *softc;
    
    //public csgesture functions
    bool ProcessMove(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    bool ProcessScroll(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    bool ProcessThreeFingerSwipe(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    
    void TapToClickOrDrag(csgesture_softc *sc, int button);
    void ClearTapDrag(csgesture_softc *sc, int i);
    void ProcessGesture(csgesture_softc *sc);
    
    //os specific functions
    void prepareToSleep();
    void wakeFromSleep();
    
    void initialize_wrapper(IOService *service);
    void destroy_wrapper(void);
    
    int vendorID;
    int productID;
    
    int reportDescriptorLength();
    void write_report_to_buffer(IOMemoryDescriptor *buffer);
    void write_report_descriptor_to_buffer(IOMemoryDescriptor *buffer);
};

#endif