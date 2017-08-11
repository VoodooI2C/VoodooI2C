//
//  VoodooI2CPCI.cpp
//  VoodooI2C
//
//  Created by Maxime Vincent on 25/06/16.
//  Copyright © 2016 Maxime Vincent. All rights reserved.
//

#include "VoodooI2CPCI.h"

#define super    IOService
OSDefineMetaClassAndStructors(VoodooI2CPCI, IOService);

bool VoodooI2CPCI::init(IOService* provider)
{
    if (!super::init()) {
        // Perform any required clean-up, then return.
        return false;
    }
    return true;
}

void VoodooI2CPCI::free(void)
{
    //deviceRegisterMap->release();
    super::free();
    return;
}


void VoodooI2CPCI::pciConfigure(IOPCIDevice* fPCIDevice)
{
    IOLog("Set PCI Power State D0\n");
    fPCIDevice->enablePCIPowerManagement(kPCIPMCSPowerStateD0);
    
    /*
     * Enable PCI device / memory response from the card
     */
    fPCIDevice->setBusMasterEnable(true);
    fPCIDevice->setMemoryEnable(true);
}

IOACPIPlatformDevice* VoodooI2CPCI::getACPIDevice(IORegistryEntry * device)
{
    IOACPIPlatformDevice *  acpiDevice = 0;
    OSString *				acpiPath;
    
    if (device)
    {
        acpiPath = (OSString *) device->copyProperty(kACPIDevicePathKey);
        if (acpiPath && !OSDynamicCast(OSString, acpiPath))
        {
            acpiPath->release();
            acpiPath = 0;
        }
        
        if (acpiPath)
        {
            IORegistryEntry * entry;
            
            // fromPath returns a retain()'d entry that needs to be released later
            entry = IORegistryEntry::fromPath(acpiPath->getCStringNoCopy());
            acpiPath->release();
            
            if (entry && entry->metaCast("IOACPIPlatformDevice"))
                acpiDevice = (IOACPIPlatformDevice *) entry;
            else if (entry)
                entry->release();
        }
    }
    
    return (acpiDevice);
}
