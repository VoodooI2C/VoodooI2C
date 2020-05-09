//
//  VoodooCSGestureHIPointingWrapper.hpp
//  VoodooI2C
//
//  Created by CoolStar on 9/6/16.
//  Copyright © 2016 CoolStar. All rights reserved.
//

#ifndef VoodooCSGestureHIPointingWrapper_h
#define VoodooCSGestureHIPointingWrapper_h

#include <IOKit/hidsystem/IOHIPointing.h>

#ifndef EXPORT
#define EXPORT __attribute__((visibility("default")))
#endif

class VoodooI2CCSGestureEngine;

class EXPORT VoodooCSGestureHIPointingWrapper : public IOHIPointing {
    typedef IOHIPointing super;
    OSDeclareDefaultStructors(VoodooCSGestureHIPointingWrapper);
    
private:
    bool horizontalScroll;
    
protected:
    IOItemCount buttonCount() override;
    IOFixed resolution() override;
    
public:
    VoodooI2CCSGestureEngine* gesturerec;
    
    bool init() override;
    
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    
    UInt32 deviceType() override;
    UInt32 interfaceID() override;
    
    IOReturn setParamProperties(OSDictionary *dict) override;
    
    void updateRelativeMouse(int dx, int dy, int buttons);
    void updateAbsoluteMouse(SInt16 x, SInt16 y, int buttons);
    void updateScroll(short dy, short dx, short dz);
};

#endif /* VoodooCSGestureHIPointingWrapper_h */
