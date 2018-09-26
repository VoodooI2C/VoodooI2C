//
//  VoodooI2CMT2PointingWrapper.cpp
//  VoodooI2C
//
//  Created by Alexandre on 26/09/2018.
//  Copyright © 2018 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CMT2PointingWrapper.hpp"

#include <IOKit/IOLib.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include "VoodooI2CMT2PointingWrapper.hpp"
#include "VoodooI2CCSGestureEngine.hpp"
#include <libkern/version.h>


OSDefineMetaClassAndStructors(VoodooI2CMT2PointingWrapper, IOHIPointing);

UInt32 VoodooI2CMT2PointingWrapper::deviceType()
{
    return NX_EVS_DEVICE_TYPE_MOUSE;
}

UInt32 VoodooI2CMT2PointingWrapper::interfaceID()
{
    return NX_EVS_DEVICE_INTERFACE_BUS_ACE;
}

IOItemCount VoodooI2CMT2PointingWrapper::buttonCount(){
    return 2;
};

IOFixed VoodooI2CMT2PointingWrapper::resolution(){
    return (300) << 16;
};

bool VoodooI2CMT2PointingWrapper::init(){
    if (!super::init())
        return false;
    
    return true;
}

bool VoodooI2CMT2PointingWrapper::start(IOService *provider){
    if (!super::start(provider))
        return false;
    
    return true;
}

void VoodooI2CMT2PointingWrapper::stop(IOService *provider){
    super::stop(provider);
}

void VoodooI2CMT2PointingWrapper::updateRelativeMouse(int dx, int dy, int buttons){
    // 0x1 = left button
    // 0x2 = right button
    // 0x4 = middle button
    
    uint64_t now_abs;
    clock_get_uptime(&now_abs);
    dispatchRelativePointerEvent(dx, dy, buttons, now_abs);
};
