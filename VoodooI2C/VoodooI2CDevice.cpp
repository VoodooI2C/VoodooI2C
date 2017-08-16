//
//  VoodooI2CDevice.cpp
//  VoodooI2C
//
//  Created by coolstar on 3/31/16.
//  Copyright (c) 2016 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CDevice.h"
#include <IOKit/IOLib.h>

OSDefineMetaClassAndStructors(VoodooI2CDevice, IOService);

bool VoodooI2CDevice::attach(IOService * provider, IOService* child)
{
    return true;
}

VoodooGPIO *VoodooI2CDevice::getGPIOController(){
    OSDictionary *match = serviceMatching("VoodooGPIO");
    OSIterator *iter = getMatchingServices(match);
    VoodooGPIO *gpioController = OSDynamicCast(VoodooGPIO, iter->getNextObject());
    
    if (gpioController != NULL){
        IOLog("%s::Got GPIO Controller! %s\n", getName(), gpioController->getName());
    }
    
    gpioController->retain();
    
    iter->release();
    
    return gpioController;
}
