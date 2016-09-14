//
//  VoodooI2CPCI.h
//  VoodooI2CPCI
//
//  Created by Maxime Vincent on 25/06/16.
//  Copyright © 2016 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CPCI_hpp
#define VoodooI2CPCI_hpp


#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>


#ifndef kACPIDevicePathKey
#define kACPIDevicePathKey			"acpi-path"
#endif


class VoodooI2CPCI : public IOService {
    OSDeclareDefaultStructors(VoodooI2CPCI);
    
public:
    bool                            init(IOService* provider);
    void                            free(void);
    static void             pciConfigure(IOPCIDevice* fPCIDevice);
    static IOACPIPlatformDevice*    getACPIDevice(IORegistryEntry * device);
};


#endif /* VoodooI2CPCIDevice_hpp */