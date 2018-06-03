//
//  VoodooCSGestureHIDEventServiceWrapper.hpp
//  VoodooI2C
//
//  Created by CoolStar on 9/6/16.
//  Copyright © 2016 CoolStar. All rights reserved.
//

#ifndef VoodooCSGestureHIDEventServiceWrapper_h
#define VoodooCSGestureHIDEventServiceWrapper_h

#include <IOKit/hidevent/IOHIDEventService.h>

class VoodooI2CCSGestureEngine;

class VoodooCSGestureHIDEventServiceWrapper : public IOHIDEventService {
    typedef IOHIDEventService super;
    OSDeclareDefaultStructors(VoodooCSGestureHIDEventServiceWrapper);
    
private:
    bool horizontalScroll;
    
    SInt16 last_x;
    SInt16 last_y;
    
public:
    VoodooI2CCSGestureEngine* gesturerec;
    
    virtual bool init() override;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    virtual IOReturn setSystemProperties(OSDictionary *dict) override;
    
    void updateRelativeMouse(int dx, int dy, int buttons);
    void updateAbsoluteMouse(SInt16 x, SInt16 y, int buttons);
    void updateScroll(short dy, short dx, short dz);
};

#endif /* VoodooCSGestureHIDEventServiceWrapper_h */
