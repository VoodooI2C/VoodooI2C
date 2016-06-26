//
//  VoodooI2CPCIDevice.hpp
//  VoodooI2CPCI
//
//  Created by Maxime Vincent on 25/06/16.
//  Copyright © 2016 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CPCIDevice_hpp
#define VoodooI2CPCIDevice_hpp


#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>


#ifndef kACPIDevicePathKey
#define kACPIDevicePathKey			"acpi-path"
#endif


class VoodooI2CPCIDevice : public IOService {
     OSDeclareDefaultStructors(VoodooI2CPCIDevice);
    
public:
    bool                            init(IOService* provider);
    void                            free(void);
    static IOPCIDevice*             pciConfigure(IOService* provider);
    static IOACPIPlatformDevice*    getACPIDevice(IORegistryEntry * device);
};


#endif /* VoodooI2CPCIDevice_hpp */
