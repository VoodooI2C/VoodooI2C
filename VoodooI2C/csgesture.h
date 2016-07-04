// CSGesture Multitouch Touchpad Library
// © 2016, CoolStar. All Rights Reserved.

#ifndef csgesturestruct_h
#define csgesturestruct_h

#include <stdint.h>
#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include "VoodooCSGestureMouseWrapper.h"

#define MAX_FINGERS 15

struct csgesture_softc {
    //hardware input
    int x[15];
    int y[15];
    int p[15];
    
    bool buttondown;
    
    //hardware info
    bool infoSetup;
    
    char product_id[16];
    char firmware_version[4];
    
    int resx;
    int resy;
    int phyx;
    int phyy;
    
    //system output
    int dx;
    int dy;
    
    int scrollx;
    int scrolly;
    
    int buttonmask;
    
    //used internally in driver
    int panningActive;
    int idForPanning;
    
    int scrollingActive;
    int idsForScrolling[2];
    int ticksSinceScrolling;
    
    int blacklistedids[15];
    
    bool mouseDownDueToTap;
    int idForMouseDown;
    bool mousedown;
    int mousebutton;
    
    int lastx[15];
    int lasty[15];
    int lastp[15];
    
    int xhistory[15][10];
    int yhistory[15][10];
    
    int flextotalx[15];
    int flextotaly[15];
    
    int totalx[15];
    int totaly[15];
    int totalp[15];
    
    int multitaskingx;
    int multitaskingy;
    int multitaskinggesturetick;
    bool multitaskingdone;
    
    int tick[15];
    int truetick[15];
    int ticksincelastrelease;
    int tickssinceclick;
};

class CSGesture {
private:
    VoodooCSGestureMouseWrapper *_wrapper;
    
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
    //public csgesture functions
    bool ProcessMove(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    bool ProcessScroll(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    bool ProcessThreeFingerSwipe(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    
    void TapToClickOrDrag(csgesture_softc *sc, int button);
    void ClearTapDrag(csgesture_softc *sc, int i);
    void ProcessGesture(csgesture_softc *sc);
    
    //os specific functions
    void initialize_wrapper(IOService *service);
    void destroy_wrapper(void);
    
    int vendorID;
    int productID;
    
    int reportDescriptorLength();
    void write_report_to_buffer(IOMemoryDescriptor *buffer);
    void write_report_descriptor_to_buffer(IOMemoryDescriptor *buffer);
};

#endif