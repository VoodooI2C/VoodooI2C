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

/**
 Evalues the ACPI power state methods

 @param enabled

 @return returns kIOReturnSuccess if successful, else returns kIOReturnNoPower
 */
IOReturn VoodooI2CACPIController::setACPIPowerState(VoodooI2CState enabled) {
    if (enabled) {
        IOLog("%s::%s Set ACPI power state _PS0\n", getName(), physical_device->name);
        if (!physical_device->acpi_device->evaluateObject("_PS0"))
            return kIOReturnNoPower;
    } else {
        IOLog("%s::%s Set ACPI power state _PS3\n", getName(), physical_device->name);
        if (!physical_device->acpi_device->evaluateObject("_PS3"))
            return kIOReturnNoPower;
    }
    return kIOReturnSuccess;
}

/**
 Called by the system's power manager to set power states
 
 @param whichState either kIOPMPowerOff or kIOPMPowerOn
 @param whatDevice Power management policy maker
 
 @return returns kIOPMAckImplied if power state has been set else maximum number of milliseconds needed for the device to be in the correct state
 */
IOReturn VoodooI2CACPIController::setPowerState(unsigned long whichState, IOService * whatDevice) {
    if (whichState == kIOPMPowerOff) {
        physical_device->awake = false;

        setACPIPowerState(kVoodooI2CStateOff);

        IOLog("%s::%s Going to sleep\n", getName(), physical_device->name);
    } else {
        if (!physical_device->awake) {
            setACPIPowerState(kVoodooI2CStateOn);

            physical_device->awake = true;
            IOLog("%s::%s Woke up\n", getName(), physical_device->name);
        }
    }
    return kIOPMAckImplied;
}

/**
 Starts the physical I2C controller
 
 @param provider IOService* representing the matched entry in the IORegistry
 
 @return returns true on succesful start, else returns false
 */

bool VoodooI2CACPIController::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }

    physical_device->acpi_device = OSDynamicCast(IOACPIPlatformDevice, provider);

    setACPIPowerState(kVoodooI2CStateOn);

    if (mapMemory() != kIOReturnSuccess) {
        IOLog("%s::%s Could not map memory\n", getName(), physical_device->name);
        return false;
    }

    if (publishNub() != kIOReturnSuccess) {
        IOLog("%s::%s Could not publish nub\n", getName(), physical_device->name);
        return false;
    }

    registerService();

    return true;
}

/**
 Stops the physical I2C controller and undoes the effects of `start`
 
 @param provider IOService* representing the matched entry in the IORegistry
 */

void VoodooI2CACPIController::stop(IOService* provider) {
    setACPIPowerState(kVoodooI2CStateOff);

    super::stop(provider);
}
