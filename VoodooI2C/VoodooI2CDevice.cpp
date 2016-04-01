//
//  VoodooI2CDevice.cpp
//  VoodooI2C
//
//  Created by coolstar on 3/31/16.
//  Copyright (c) 2016 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CDevice.h"

OSDefineMetaClassAndStructors(VoodooI2CDevice, IOService);

bool VoodooI2CDevice::attach(IOService * provider, IOService* child)
{
    return true;
}