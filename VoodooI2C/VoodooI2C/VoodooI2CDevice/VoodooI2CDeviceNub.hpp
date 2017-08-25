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
#include "../../../Dependencies/VoodooGPIO/VoodooGPIO/VoodooGPIO.h"

class VoodooI2CDeviceNub : public IOService {
  OSDeclareDefaultStructors(VoodooI2CDeviceNub);

 public:
    bool attach(IOService* provider, IOService* child);
    IOReturn disableInterrupt(int source);
    IOReturn enableInterrupt(int source);
    IOReturn getInterruptType(int source, int *interruptType);
    IOReturn registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refcon);
    bool start(IOService* provider);
    void stop(IOService* provider);
    IOReturn unregisterInterrupt(int source);

 private:
    IOACPIPlatformDevice* acpi_device;
    const char* controller_name;
    VoodooGPIO* gpio_controller;
    int gpio_irq;
    UInt16 gpio_pin;
    bool has_gpio_interrupts;
    
    IOReturn getDeviceResources();
    VoodooGPIO* getGPIOController();
};


#endif /* VoodooI2CDeviceNub_hpp */
