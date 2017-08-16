//
//  VoodooI2CDevice.h
//  VoodooI2C
//
//  Created by coolstar on 3/31/16.
//  Copyright (c) 2016 Alexandre Daoud. All rights reserved.
//

#ifndef __VoodooI2C__VoodooI2CDevice__
#define __VoodooI2C__VoodooI2CDevice__

#include <IOKit/IOService.h>
#include "linuxirq.h"
#include "VoodooGPIO.h"

class VoodooI2CDevice : public IOService
{
    OSDeclareDefaultStructors(VoodooI2CDevice);
public:
    virtual bool attach(IOService * provider, IOService* child);
    
    VoodooGPIO *getGPIOController();
};

#endif /* defined(__VoodooI2C__VoodooI2CDevice__) */
