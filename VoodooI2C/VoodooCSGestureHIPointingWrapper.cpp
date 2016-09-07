//
//  VoodooCSGestureHIPointingWrapper.cpp
//  VoodooI2C
//
//  Created by CoolStar on 9/6/16.
//  Copyright © 2016 CoolStar. All rights reserved.
//

#include <IOKit/IOLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include "VoodooCSGestureHIPointingWrapper.h"

OSDefineMetaClassAndStructors(VoodooCSGestureHIPointingWrapper, IOHIPointing);

UInt32 VoodooCSGestureHIPointingWrapper::deviceType()
{
    return NX_EVS_DEVICE_TYPE_MOUSE;
}

UInt32 VoodooCSGestureHIPointingWrapper::interfaceID()
{
    return NX_EVS_DEVICE_INTERFACE_BUS_ACE;
}

IOItemCount VoodooCSGestureHIPointingWrapper::buttonCount(){
    return 2;
};

IOFixed VoodooCSGestureHIPointingWrapper::resolution(){
    return (300) << 16;
};

bool VoodooCSGestureHIPointingWrapper::init(){
    if (!super::init())
        return false;
    
    return true;
}

bool VoodooCSGestureHIPointingWrapper::start(IOService *provider){
    if (!super::start(provider))
        return false;
    
    int enabledProperty = 1;
    setProperty("Clicking", enabledProperty,
                sizeof(enabledProperty) * 8);
    setProperty("TrackpadScroll", enabledProperty,
                sizeof(enabledProperty) * 8);
    setProperty("TrackpadHorizScroll", enabledProperty,
                sizeof(enabledProperty) * 8);
    
    //
    // Must add this property to let our superclass know that it should handle
    // trackpad acceleration settings from user space.  Without this, tracking
    // speed adjustments from the mouse prefs panel have no effect.
    //
    
    int prodID = 547;
    setProperty("ProductID", prodID, sizeof(prodID) * 8);
    
    int vendorID = 1452;
    setProperty("VendorID", vendorID, sizeof(vendorID) * 8);
    
    //Need to fake Vendor and Product ID to pull up the trackpad settings pane
    
    setProperty(kIOHIDPointerAccelerationTypeKey, kIOHIDTrackpadAccelerationType);
    setProperty(kIOHIDScrollAccelerationTypeKey, kIOHIDTrackpadScrollAccelerationKey);
    setProperty(kIOHIDScrollResolutionKey, 800 << 16, 32);
    
    return true;
}

void VoodooCSGestureHIPointingWrapper::stop(IOService *provider){
    super::stop(provider);
}

void VoodooCSGestureHIPointingWrapper::updateRelativeMouse(int dx, int dy, int buttons){
    // 0x1 = left button
    // 0x2 = right button
    // 0x4 = middle button
    
    uint64_t now_abs;
    clock_get_uptime(&now_abs);
    dispatchRelativePointerEvent(dx, dy, buttons, now_abs);
}

void VoodooCSGestureHIPointingWrapper::updateScroll(short dy, short dx, short dz){
    uint64_t now_abs;
    clock_get_uptime(&now_abs);
    dispatchScrollWheelEvent(dy, dx, dz, now_abs);
}

IOReturn VoodooCSGestureHIPointingWrapper::setParamProperties(OSDictionary *dict)
{
    OSNumber *clicking = OSDynamicCast(OSNumber, dict->getObject("Clicking"));
    OSNumber *dragging = OSDynamicCast(OSNumber, dict->getObject("Dragging"));
    OSNumber *draglock = OSDynamicCast(OSNumber, dict->getObject("DragLock"));
    OSNumber *hscroll  = OSDynamicCast(OSNumber, dict->getObject("TrackpadHorizScroll"));
    OSNumber *vscroll  = OSDynamicCast(OSNumber, dict->getObject("TrackpadScroll"));
    OSNumber *eaccell  = OSDynamicCast(OSNumber, dict->getObject("HIDTrackpadScrollAcceleration"));
    
    return super::setParamProperties(dict);
}

