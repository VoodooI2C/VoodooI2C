//
//  VoodooI2CPCIController.cpp
//  VoodooI2C
//
//  Created by Alexandre on 02/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CPCIController.hpp"

#define super VoodooI2CController
OSDefineMetaClassAndStructors(VoodooI2CPCIController, VoodooI2CController);

void VoodooI2CPCIController::configurePCI() {
    IOLog("%s::%s Set PCI power state D0\n", getName(), physical_device->name);
    physical_device->pci_device->enablePCIPowerManagement(kPCIPMCSPowerStateD0);

    physical_device->pci_device->setBusMasterEnable(true);
    physical_device->pci_device->setMemoryEnable(true);
}

IOReturn VoodooI2CPCIController::getACPIDevice() {
    OSString*  acpi_path;

    acpi_path = reinterpret_cast<OSString*>(physical_device->pci_device->copyProperty(kACPIDevicePathKey));
    if (acpi_path && !OSDynamicCast(OSString, acpi_path)) {
        acpi_path->release();
        return kIOReturnError;
    }

    if (acpi_path) {
        IORegistryEntry* entry;

        // fromPath returns a retain()'d entry that needs to be released later
        entry = IORegistryEntry::fromPath(acpi_path->getCStringNoCopy());
        acpi_path->release();

        if (entry && entry->metaCast("IOACPIPlatformDevice"))
            physical_device->acpi_device = reinterpret_cast<IOACPIPlatformDevice*>(entry);
        else if (entry)
            entry->release();
    }

    return kIOReturnSuccess;
}

IOReturn VoodooI2CPCIController::setPowerState(unsigned long whichState, IOService* whatDevice) {
    if (whatDevice != this)
        return kIOPMAckImplied;

    if (whichState == kIOPMPowerOff) {
        physical_device->awake = false;

        IOLog("%s::%s Going to sleep\n", getName(), physical_device->name);
    } else {
        if (!physical_device->awake) {
            configurePCI();
            skylakeLPSSResetHack();

            physical_device->awake = true;
            IOLog("%s::%s Woke up\n", getName(), physical_device->name);
        }
    }
    return kIOPMAckImplied;
}

void VoodooI2CPCIController::skylakeLPSSResetHack() {
    writeRegister((LPSS_PRIV_RESETS_FUNC | LPSS_PRIV_RESETS_IDMA), (LPSS_PRIV + LPSS_PRIV_RESETS));
}

bool VoodooI2CPCIController::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }

    physical_device->pci_device = OSDynamicCast(IOPCIDevice, provider);

    if (getACPIDevice() != kIOReturnSuccess) {
        IOLog("%s::%s Could not get ACPI device for PCI provider", getName(), physical_device->name);
        return false;
    }

    configurePCI();

    if (mapMemory() != kIOReturnSuccess) {
        IOLog("%s::%s Could not map memory\n", getName(), physical_device->name);
        return false;
    }

    skylakeLPSSResetHack();

    if (publishNub() != kIOReturnSuccess) {
        IOLog("%s::%s Could not publish nub\n", getName(), physical_device->name);
        return false;
    }

    registerService();

    return true;
}

void VoodooI2CPCIController::stop(IOService* provider) {
    super::stop(provider);
}
