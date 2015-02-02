//
//  VoodooI2CDevice.h
//  VoodooI2C
//
//  Created by Alexandre on 02/02/2015.
//  Copyright (c) 2015 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2C_VoodooI2CHIDDevice_h
#define VoodooI2C_VoodooI2CHIDDevice_h

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>

class VoodooI2C;

class VoodooI2CHIDDevice : public IOService
{
    typedef IOService super;
    OSDeclareDefaultStructors(VoodooI2CHIDDevice);
    
protected:
    VoodooI2C* _controller;
    
public:
    virtual bool attach(IOService * provider, IOService* child);
    virtual void detach(IOService * provider);
};


#endif
