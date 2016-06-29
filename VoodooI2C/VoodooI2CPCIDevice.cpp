//
//  VoodooI2CPCIDevice.cpp
//  VoodooI2CPCI
//
//  Created by Maxime Vincent on 25/06/16.
//  Copyright © 2016 Maxime Vincent. All rights reserved.
//

#include "VoodooI2CPCIDevice.h"

#define super    IOService
OSDefineMetaClassAndStructors(VoodooI2CPCIDevice, IOService);

bool VoodooI2CPCIDevice::init(IOService* provider)
{
    if (!super::init()) {
        // Perform any required clean-up, then return.
        return false;
    }
    return true;
}

void VoodooI2CPCIDevice::free(void)
{
    //deviceRegisterMap->release();
    super::free();
    return;
}


IOPCIDevice* VoodooI2CPCIDevice::pciConfigure(IOService* provider)
{
    IOPCIDevice *fPCIDevice = NULL;
    
    fPCIDevice = OSDynamicCast(IOPCIDevice, provider);
    if (!fPCIDevice)
        return NULL;

    if (!fPCIDevice->open(provider)) {
        IOLog("[VoodooI2CPCI]: Failed to open provider.\n");
        return NULL;
    }
    
    IOLog("Set PCI Power State D0\n");
    fPCIDevice->enablePCIPowerManagement(kPCIPMCSPowerStateD0);
    
    /*
     * Enable PCI device / memory response from the card
     */
    fPCIDevice->setBusMasterEnable(true);
    fPCIDevice->setMemoryEnable(true);
    
    return fPCIDevice;
}

IOACPIPlatformDevice* VoodooI2CPCIDevice::getACPIDevice(IORegistryEntry * device)
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