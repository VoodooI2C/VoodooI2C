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
#include "csgesture.h"

class VoodooCSGestureHIPointingWrapper : public IOHIPointing
{
    typedef IOHIPointing super;
    OSDeclareDefaultStructors(VoodooCSGestureHIPointingWrapper);
    
private:
    bool horizontalScroll;
    
protected:
    virtual IOItemCount buttonCount();
    virtual IOFixed resolution();
    
public:
    CSGesture *gesturerec;
    
    virtual bool init();
    
    virtual bool start(IOService *provider);
    virtual void stop(IOService *provider);
    
    virtual UInt32 deviceType();
    virtual UInt32 interfaceID();
    
    virtual IOReturn setParamProperties(OSDictionary *dict);
    
    void updateRelativeMouse(int dx, int dy, int buttons);
    void updateScroll(short dy, short dx, short dz);
};

#endif /* VoodooCSGestureHIPointingWrapper_h */
