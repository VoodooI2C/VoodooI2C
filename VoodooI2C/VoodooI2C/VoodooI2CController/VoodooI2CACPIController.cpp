//
//  VoodooI2CACPIController.cpp
//  VoodooI2C
//
//  Created by Alexandre on 02/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CACPIController.hpp"

#define super VoodooI2CController
OSDefineMetaClassAndStructors(VoodooI2CACPIController, VoodooI2CController);

IOReturn VoodooI2CACPIController::setACPIPowerState(VoodooI2CState enabled) {
    if (enabled) {
        IOLog("%s::%s Set ACPI power state _PS0\n", getName(), physical_device.name);
        if (!physical_device.acpi_device->evaluateObject("_PS0"))
            return kIOReturnNoPower;
    } else {
        IOLog("%s::%s Set ACPI power state _PS3\n", getName(), physical_device.name);
        if (!physical_device.acpi_device->evaluateObject("_PS3"))
            return kIOReturnNoPower;
    }
    return kIOReturnSuccess;
}

IOReturn VoodooI2CACPIController::setPowerState(unsigned long whichState, IOService * whatDevice) {
    if (whatDevice != this)
        return kIOPMAckImplied;

    if (whichState == 0) {  // index of kIOPMPowerOff state in VoodooI2CIOPMPowerStates
        if (physical_device.awake) {
            physical_device.awake = false;
            unmapMemory();
            setACPIPowerState(kVoodooI2CStateOff);
            IOLog("%s::%s Going to sleep\n", getName(), physical_device.name);
        }
    } else {
        if (!physical_device.awake) {
            setACPIPowerState(kVoodooI2CStateOn);
            if (mapMemory() != kIOReturnSuccess)
                IOLog("%s::%s Could not map memory\n", getName(), physical_device.name);
            physical_device.awake = true;
            IOLog("%s::%s Woke up\n", getName(), physical_device.name);
        }
    }
    return kIOPMAckImplied;
}

bool VoodooI2CACPIController::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }

    physical_device.acpi_device = OSDynamicCast(IOACPIPlatformDevice, provider);

    OSBoolean *access_intr_mask_workaround  = OSDynamicCast(OSBoolean, this->getProperty("AccessIntrMaskWorkaround"));
    if (access_intr_mask_workaround) {
        physical_device.access_intr_mask_workaround = access_intr_mask_workaround->getValue();
        IOLog("%s::%s AccessIntrMaskWorkaround: %d\n", getName(), physical_device.name, physical_device.access_intr_mask_workaround);
    }

    setACPIPowerState(kVoodooI2CStateOn);

    if (mapMemory() != kIOReturnSuccess) {
        IOLog("%s::%s Could not map memory\n", getName(), physical_device.name);
        return false;
    }

    if (publishNub() != kIOReturnSuccess) {
        IOLog("%s::%s Could not publish nub\n", getName(), physical_device.name);
        return false;
    }

    registerService();

    return true;
}

void VoodooI2CACPIController::stop(IOService* provider) {
    setACPIPowerState(kVoodooI2CStateOff);

    super::stop(provider);
}
