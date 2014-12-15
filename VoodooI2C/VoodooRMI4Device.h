//
//  RMI4Device.h
//  VoodooI2C
//
//  Created by Alexandre on 12/12/2014.
//  Copyright (c) 2014 Alexandre Daoud. All rights reserved.
//

#ifndef __VoodooI2C__VoodooRMI4Device__
#define __VoodooI2C__VoodooRMI4Device__

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOCommandGate.h>
//


#endif /* defined(__VoodooI2C__RMI4Device__) */

class VoodooI2C;

class VoodooRMI4Device : public IOService {
    
    OSDeclareDefaultStructors(VoodooRMI4Device);
    
    friend class VoodooI2C;
    
public:
    
    typedef struct {
        int page;
    } rmi_i2c_data;
    
    typedef struct {
        
        unsigned short addr;
        
        rmi_i2c_data *data;
        
        char* name;
        
        VoodooI2C* bus_class;
        
        IOACPIPlatformDevice* provider;
        
    } RMI4Device;
    
    RMI4Device* _rmi4dev;
    
    bool start(IOService* provider);
    void stop();
};
