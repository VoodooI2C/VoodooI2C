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

void VoodooI2CControllerNub::interruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount) {
    if (driver)
        driver->handleInterrupt();
}

UInt32 VoodooI2CControllerNub::readRegister(int offset) {
    return controller->readRegister(offset);
}

/**
 Releases the driver resources retained by `start`
 */

void VoodooI2CControllerNub::releaseResources() {
    if (command_gate) {
        work_loop->removeEventSource(command_gate);
        command_gate->release();
        command_gate = NULL;
    }

    if (interrupt_source) {
        interrupt_source->disable();
        work_loop->removeEventSource(interrupt_source);
        interrupt_source->release();
        interrupt_source = NULL;
    }

    if (work_loop)
        OSSafeReleaseNULL(work_loop);
}

bool VoodooI2CControllerNub::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    PMinit();

    work_loop = reinterpret_cast<IOWorkLoop*>(getWorkLoop());
    if (!work_loop) {
        IOLog("%s::%s Could not get work loop\n", getName(), name);
        goto exit;
    }

    interrupt_source =
    IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CControllerNub::interruptOccured), controller->physical_device->provider);

    if (!interrupt_source || work_loop->addEventSource(interrupt_source) != kIOReturnSuccess) {
        IOLog("%s::%s::Could not add interrupt source to work loop\n", getName(), name);
        goto exit;
    }

    interrupt_source->enable();

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

void VoodooI2CControllerNub::stop(IOService* provider) {
    releaseResources();
    super::stop(provider);
}

void VoodooI2CControllerNub::writeRegister(UInt32 value, int offset) {
    controller->writeRegister(value, offset);
}
