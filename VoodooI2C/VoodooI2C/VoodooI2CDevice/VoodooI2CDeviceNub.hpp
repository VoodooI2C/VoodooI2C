//
//  VoodooI2CDeviceNub.hpp
//  VoodooI2C
//
//  Created by Alexandre on 07/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CDeviceNub_hpp
#define VoodooI2CDeviceNub_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include "VoodooGPIO.hpp"

class VoodooI2CDeviceNub : public IOService {
  OSDeclareDefaultStructors(VoodooI2CDeviceNub);

 public:
    // data members

    // function members
    bool attach(IOService* provider, IOService* child);
    void detach(IOService* provider);
    bool init(OSDictionary* properties);
    void free();
    bool start(IOService* provider);
    void stop(IOService* provider);
    
    virtual IOReturn disableInterrupt(int source) override;
    virtual IOReturn enableInterrupt(int source) override;
    virtual IOReturn getInterruptType(int source, int *interruptType) override;
    virtual IOReturn registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refcon) override;
    virtual IOReturn unregisterInterrupt(int source);
 protected:
 private:
    IOACPIPlatformDevice *acpi_device;
    bool get_device_resources();
    
    VoodooGPIO *gpioController;
    VoodooGPIO *getGPIOController();
    
    bool hasGPIOInt;
    
    UInt16 gpioPin;
    int gpioIRQ;
};


#endif /* VoodooI2CDeviceNub_hpp */
