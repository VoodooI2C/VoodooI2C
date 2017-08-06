//
//  VoodooI2CControllerNub.cpp
//  VoodooI2C
//
//  Created by Alexandre on 03/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CController.hpp"
#include "VoodooI2CControllerNub.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CControllerNub, IOService);

/**
 Attaches the nub to the physical I2C controller
 
 @param provider IOService* representing the provider
 
 @return returns true on succesful attach, else returns false
 */

bool VoodooI2CControllerNub::attach(IOService* provider) {
    if (!super::attach(provider)) {
        IOLog("%s super::attach failed", getName());
    }

    controller = OSDynamicCast(VoodooI2CController, provider);
    controller->retain();

    name = controller->physical_device->name;

    if (!controller)
        return false;

    return true;
}

/**
 Detaches the nub from the physical I2C controller
 
 @param provider IOService* representing the provider
 */

void VoodooI2CControllerNub::detach(IOService* provider) {
    if (controller) {
        controller->release();
        controller = NULL;
    }

    super::detach(provider);
}

/**
 Evaluates ACPI methods pertaining to the ACPI device for the controller in the ACPI tables

 @param method   const char* containg the method name
 @param hcnt     pointer to UInt32 containg the high count
 @param lcnt     pointer to UInt32 containg the low count
 @param sda_hold pointer to sda_hold containing the SDA hold time

 @return returns kIOReturnSuccess on successful retrieval of all desired values,
         else returns kIOReturnNotFound
 */
IOReturn VoodooI2CControllerNub::getACPIParams(const char* method, UInt32* hcnt, UInt32* lcnt, UInt32* sda_hold) {
    OSObject *object;
    IOReturn status = controller->physical_device->acpi_device->evaluateObject(method, &object);

    if (status == kIOReturnSuccess && object) {
        OSArray* values = OSDynamicCast(OSArray, object);
        if (!values) {
            IOLog("%s::%s %s not implemented in ACPI tables\n", getName(), name, method);
            return kIOReturnNotFound;
        }

        OSNumber *hcntNum = OSDynamicCast(OSNumber, values->getObject(0));
        if (hcntNum)
            *hcnt = hcntNum->unsigned32BitValue();
        else
            IOLog("%s::%s Warning: %s HCNT not implemented in ACPI tables\n", getName(), name, method);

        OSNumber *lcntNum = OSDynamicCast(OSNumber, values->getObject(1));
        if (lcntNum)
            *lcnt = lcntNum->unsigned32BitValue();
        else
            IOLog("%s::%s Warning: %s LCNT not implemented in ACPI tables\n", getName(), name, method);

        if (sda_hold) {
            OSNumber *sdaHoldNum = OSDynamicCast(OSNumber, values->getObject(2));
            if (sdaHoldNum)
                *sda_hold = sdaHoldNum->unsigned32BitValue();
            else
                IOLog("%s::%s Warning: %s SDA hold not implemented in ACPI tables\n", getName(), name, method);
        }

    } else {
        IOLog("%s::%s %s not implemented in ACPI tables\n", getName(), name, method);
        return kIOReturnNotFound;
    }

    OSSafeReleaseNULL(object);

    return kIOReturnSuccess;
exit:
    OSSafeReleaseNULL(object);
    return kIOReturnNotFound;
}

/**
 Passes to the `readRegister` command in `VoodooI2CController`
 */

UInt32 VoodooI2CControllerNub::readRegister(int offset) {
    return controller->readRegister(offset);
}

void VoodooI2CControllerNub::releaseResources() {
    if (command_gate) {
        work_loop->removeEventSource(command_gate);
        command_gate->release();
        command_gate = NULL;
    }

    if (work_loop)
        OSSafeReleaseNULL(work_loop);
}

/**
 Starts the controller nub
 
 @param provider IOService* representing the physical controller
 @return returns true on succesful start, else returns false
 */

bool VoodooI2CControllerNub::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    work_loop = reinterpret_cast<IOWorkLoop*>(getWorkLoop());
    if (!work_loop) {
        IOLog("%s::%s Could not get work loop\n", getName(), name);
        goto exit;
    }

    command_gate = IOCommandGate::commandGate(this);
    if (!command_gate || (work_loop->addEventSource(command_gate) != kIOReturnSuccess)) {
        IOLog("%s::%s Could not open command gate\n", getName(), name);
        goto exit;
    }

    work_loop->retain();

    return true;

exit:
    releaseResources();
    return false;
}

/**
 Stops the physical I2C controller and undoes the effects of `start` and `probe`
 
 @param provider IOService* representing the matched entry in the IORegistry
 */

void VoodooI2CControllerNub::stop(IOService* provider) {
    releaseResources();
    super::stop(provider);
}

/**
 Passes to the `writeRegister` command in `VoodooI2CController`
 */

void VoodooI2CControllerNub::writeRegister(UInt32 value, int offset) {
    controller->writeRegister(value, offset);
}
