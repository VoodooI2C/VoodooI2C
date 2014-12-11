
//
//  VoodooACPII2CNub.h
//  VoodooI2C
//
//  Created by Alexandre on 10/12/2014.
//  Copyright (c) 2014 Alexandre Daoud. All rights reserved.
//

#ifndef __VoodooI2C__VoodooACPII2CNub__
#define __VoodooI2C__VoodooACPII2CNub__

#include <IOKit/IOService.h>
#include <IOkit/IOPlatformExpert.h>
#include <IOKit/acpi/IOACPITypes.h>

#define EXPORT __attribute__((visibility("default");

class IOPlatformExpert;

class VoodooACPII2CNub : public IOService {
    typedef IOService super;
    
    OSDeclareDefaultStructors(VoodooACPII2CNub);
    
public:
    virtual bool start(IOService* provider);
    virtual void stop(IOService* provider);
};

#endif /* defined(__VoodooI2C__VoodooACPII2CNub__) */
