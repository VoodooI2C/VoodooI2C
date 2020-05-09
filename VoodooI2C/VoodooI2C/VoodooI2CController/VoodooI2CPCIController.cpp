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
    char tmp[2];
    const char kCometLakeflag[2] = {'2', 'e'};

    IOLog("%s::%s Set PCI power state D0\n", getName(), physical_device.name);
    auto pci_device = physical_device.pci_device;
    pci_device->enablePCIPowerManagement(kPCIPMCSPowerStateD0);

    /* To apply this patch, we need to check if it's 10th Comet Lake CPU
       because this hack patch can't work in other platforms like 8th Kaby Lake R.
       Every 10th CPU 's id includes "2e" in the index 8 and index9, which can be used to check the
       platform. Thx for @Williambj1 's discovery.*/

    OSString *mystring = OSString::withCString(physical_device.name);
    if (!mystring) {
        IOLog("%s::%s Get IONameMatched data error!\n", getName(), physical_device.name);
        return;
    }

    // Write your computer 's id flag for comparision
    tmp[0] = mystring->getChar(8);
    tmp[1] = mystring->getChar(9);

    OSSafeReleaseNULL(mystring);

    /* If it is Comet Lake, then let's apply Forcing D0 here.
       It will modify 0x80 below to your findings.*/

    if (tmp[0] == kCometLakeflag[0] && tmp[1] == kCometLakeflag[1]) {
        IOLog("%s::%s Current CPU is Comet Lake, patching...\n", getName(), physical_device.name);
        uint16_t oldPowerStateWord = pci_device->configRead16(0x80 + 0x4);
        uint16_t newPowerStateWord = (oldPowerStateWord & (~0x3)) | 0x0;
        // Modify 0x80 below to your findings.
        pci_device->configWrite16(0x80 + 0x4, newPowerStateWord);
        IOLog("%s::%s Successfully patched!\n", getName(), physical_device.name);
    }

    pci_device->setBusMasterEnable(true);
    pci_device->setMemoryEnable(true);
}

IOReturn VoodooI2CPCIController::getACPIDevice() {
    // Get ACPI device path
    OSObject* acpi_path_object = physical_device.pci_device->copyProperty(kACPIDevicePathKey);
    OSString* acpi_path = OSDynamicCast(OSString, acpi_path_object);
    if (!acpi_path) {
        OSSafeReleaseNULL(acpi_path_object);
        return kIOReturnError;
    }

    // Get ACPI device
    IORegistryEntry* entry = IORegistryEntry::fromPath(acpi_path->getCStringNoCopy());
    acpi_path->release();

    physical_device.acpi_device = OSDynamicCast(IOACPIPlatformDevice, entry);
    if (!physical_device.acpi_device) {
        OSSafeReleaseNULL(entry);
        return kIOReturnError;
    }

    return kIOReturnSuccess;
}

IOReturn VoodooI2CPCIController::setPowerState(unsigned long whichState, IOService* whatDevice) {
    if (whatDevice != this)
        return kIOPMAckImplied;

    if (whichState == kIOPMPowerOff) {
        physical_device.awake = false;
        unmapMemory();
        IOLog("%s::%s Going to sleep\n", getName(), physical_device.name);
    } else {
        if (!physical_device.awake) {
            configurePCI();
            if (mapMemory() != kIOReturnSuccess)
                IOLog("%s::%s Could not map memory\n", getName(), physical_device.name);
            skylakeLPSSResetHack();

            physical_device.awake = true;
            IOLog("%s::%s Woke up\n", getName(), physical_device.name);
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

    physical_device.pci_device = OSDynamicCast(IOPCIDevice, provider);

    if (getACPIDevice() != kIOReturnSuccess) {
        IOLog("%s::%s Could not get ACPI device for PCI provider", getName(), physical_device.name);
        return false;
    }

    configurePCI();

    if (mapMemory() != kIOReturnSuccess) {
        IOLog("%s::%s Could not map memory\n", getName(), physical_device.name);
        return false;
    }

    skylakeLPSSResetHack();

    if (publishNub() != kIOReturnSuccess) {
        IOLog("%s::%s Could not publish nub\n", getName(), physical_device.name);
        return false;
    }

    registerService();

    return true;
}

void VoodooI2CPCIController::stop(IOService* provider) {
    OSSafeReleaseNULL(physical_device.acpi_device);
    super::stop(provider);
}
