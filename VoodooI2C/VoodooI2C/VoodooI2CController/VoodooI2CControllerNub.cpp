//
//  VoodooI2CControllerNub.cpp
//  VoodooI2C
//
//  Created by Alexandre on 03/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CController.hpp"
#include "VoodooI2CControllerNub.hpp"
#include "VoodooI2CControllerDriver.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CControllerNub, IOService);

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

void VoodooI2CControllerNub::detach(IOService* provider) {
    if (controller) {
        controller->release();
        controller = NULL;
    }

    super::detach(provider);
}

IOReturn VoodooI2CControllerNub::disableInterrupt(int source) {
    return controller->physical_device->provider->disableInterrupt(source);
}

IOReturn VoodooI2CControllerNub::enableInterrupt(int source) {
    return controller->physical_device->provider->enableInterrupt(source);
}

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

IOReturn VoodooI2CControllerNub::getInterruptType(int source, int *interruptType) {
    return controller->physical_device->provider->getInterruptType(source, interruptType);
}

UInt32 VoodooI2CControllerNub::readRegister(int offset) {
    return controller->readRegister(offset);
}

IOReturn VoodooI2CControllerNub::registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refcon) {
    return controller->physical_device->provider->registerInterrupt(source, target, handler, refcon);
}

bool VoodooI2CControllerNub::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    return true;
}

void VoodooI2CControllerNub::stop(IOService* provider) {
    super::stop(provider);
}

IOReturn VoodooI2CControllerNub::unregisterInterrupt(int source) {
    return controller->physical_device->provider->unregisterInterrupt(source);
}

/**
 Passes to the `writeRegister` command in `VoodooI2CController`
 */

void VoodooI2CControllerNub::writeRegister(UInt32 value, int offset) {
    controller->writeRegister(value, offset);
}
