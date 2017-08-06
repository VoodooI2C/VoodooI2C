//
//  VoodooI2CControllerDriver.hpp
//  VoodooI2C
//
//  Created by Alexandre on 03/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CControllerDriver_hpp
#define VoodooI2CControllerDriver_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#include "VoodooI2CControllerConstants.hpp"
#include "VoodooI2CControllerNub.hpp"
#include "../../../Miscellaneous/helpers.hpp"

// The default values in the following struct are for Haswell PCI as
// specified in Linux::i2c-designware-pcidrv.c

typedef struct {
    UInt32 ss_hcnt = 0x01b0;
    UInt32 fs_hcnt = 0x48;
    UInt32 ss_lcnt = 0x01fb;
    UInt32 fs_lcnt = 0xa0;
    UInt32 sda_hold = 0x9;
} VoodooI2CControllerBusConfig;

typedef struct {
    const char* name;
    VoodooI2CControllerBusConfig* acpi_config;
    UInt32 functionality;
    UInt32 bus_config;
    UInt rx_fifo_depth = 32;
    UInt tx_fifo_depth = 32;
} VoodooI2CControllerBusDevice;

class VoodooI2CControllerDriver : public IOService {
  OSDeclareDefaultStructors(VoodooI2CControllerDriver);

 public:
    // data members

    VoodooI2CControllerBusDevice* bus_device;
    VoodooI2CControllerNub* nub;

    // function members
    // bool attach(IOService* provider);
    // void detach(IOService* provider);
    VoodooI2CControllerDriver* probe(IOService* provider, SInt32* score);
    bool init(OSDictionary* properties);
    void free();
    bool start(IOService* provider);
    // void stop(IOService* provider);

 protected:
 private:
    IOReturn toggleBusState(VoodooI2CPowerState enabled);
    inline void toggleClockGating(VoodooI2CPowerState enabled);
    IOReturn getBusConfig();
    IOReturn initialiseBus();
};


#endif /* VoodooI2CControllerDriver_hpp */
