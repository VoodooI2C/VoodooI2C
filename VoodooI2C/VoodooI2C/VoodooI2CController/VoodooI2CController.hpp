//
//  VoodooI2CController.hpp
//
//  Created by Alexandre on 31/07/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CController_hpp
#define VoodooI2CController_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/pci/IOPCIDevice.h>

#include "../../../Miscellaneous/helpers.hpp"

#ifndef kACPIDevicePathKey
#define kACPIDevicePathKey "acpi-path"
#endif

typedef struct {
    IOACPIPlatformDevice* acpi_device;
    bool awake = true;
    const char* name;
    IOPCIDevice* pci_device;
    IOMemoryMap* mmap;
    IOService* provider;
} VoodooI2CControllerPhysicalDevice;

class VoodooI2CControllerNub;

class VoodooI2CController : public IOService {
  OSDeclareDefaultStructors(VoodooI2CController);

 public:
    // function members

    virtual VoodooI2CController* probe(IOService* provider, SInt32* score);
    UInt32 readRegister(int offset);
    virtual bool start(IOService* provider);
    virtual void stop(IOService* provider);
    void writeRegister(UInt32 value, int offset);

    // data members

    VoodooI2CControllerNub* nub;
    VoodooI2CControllerPhysicalDevice* physical_device;

 protected:
    IOReturn mapMemory();
    IOReturn publishNub();
    // void releaseMemory();

 private:
    // data members

    bool debug_logging = true;

    // function memmbers

    virtual void free();
    virtual bool init(OSDictionary* properties);
    void releaseResources();
    virtual IOReturn setPowerState(unsigned long whichState, IOService * whatDevice);
};

#endif /* VoodooI2CController_hpp */
