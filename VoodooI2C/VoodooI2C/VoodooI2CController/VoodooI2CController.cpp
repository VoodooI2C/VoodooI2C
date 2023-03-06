//
//  VoodooI2CController.cpp
//
//  Created by Alexandre on 31/07/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CController.hpp"
#include "VoodooI2CControllerNub.hpp"

// Log only if current thread is interruptible, otherwise we will get a panic.
#define TryLog(args...) do { if (ml_get_interrupts_enabled()) IOLog(args); } while (0)

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CController, IOService);

void VoodooI2CController::free() {
    super::free();
}

bool VoodooI2CController::init(OSDictionary* properties) {
    if (!super::init(properties)) {
        if (debug_logging)
            IOLog("%s super::init failed\n", getName());
        return false;
    }

    memset(&physical_device, 0, sizeof(VoodooI2CControllerPhysicalDevice));
    physical_device.awake = true;

    return true;
}

IOReturn VoodooI2CController::mapMemory() {
    if (physical_device.provider->getDeviceMemoryCount() == 0) {
        return kIOReturnDeviceError;
    } else {
        physical_device.mmap = physical_device.provider->mapDeviceMemoryWithIndex(0);
        if (!physical_device.mmap) return kIOReturnDeviceError;
        return kIOReturnSuccess;
    }
}

IOReturn VoodooI2CController::unmapMemory() {
    OSSafeReleaseNULL(physical_device.mmap);
    return kIOReturnSuccess;
}

VoodooI2CController* VoodooI2CController::probe(IOService* provider, SInt32* score) {
    if (!super::probe(provider, score)) {
        if (debug_logging)
            IOLog("%s::%s super::probe failed\n", getName(), getMatchedName(provider));
        return NULL;
    }

    return this;
}

IOReturn VoodooI2CController::publishNub() {
    IOLog("%s::%s Publishing nub\n", getName(), physical_device.name);
    nub = OSTypeAlloc(VoodooI2CControllerNub);

    if (!nub || !nub->init()) {
        IOLog("%s::%s Could not initialise nub\n", getName(), physical_device.name);
        goto exit;
    }

    if (!nub->attach(this)) {
        IOLog("%s::%s Could not attach nub\n", getName(), physical_device.name);
        goto exit;
    }

    if (!nub->start(this)) {
        IOLog("%s::%s Could not start nub\n", getName(), physical_device.name);
        nub->detach(this);
        goto exit;
    }

    setProperty("VoodooI2CServices Supported", kOSBooleanTrue);
    return kIOReturnSuccess;

exit:
    OSSafeReleaseNULL(nub);
    return kIOReturnError;
}

UInt32 VoodooI2CController::readRegister(int offset) {
    if (physical_device.mmap != 0) {
         IOVirtualAddress address = physical_device.mmap->getVirtualAddress();
         if (address != 0)
             return *(const volatile UInt32 *)(address + offset);
         else
             TryLog("%s::%s readRegister at offset 0x%x failed to get a virtual address\n", getName(), physical_device.name, offset);
     } else {
         TryLog("%s::%s readRegister at offset 0x%x failed since mamory was not mapped\n", getName(), physical_device.name, offset);
     }
     return 0;
}

void VoodooI2CController::releaseResources() {
    if (nub) {
        nub->stop(this);
        nub->detach(this);
        OSSafeReleaseNULL(nub);
    }

    OSSafeReleaseNULL(physical_device.mmap);

    if (physical_device.provider) {
        physical_device.provider->close(this);
        OSSafeReleaseNULL(physical_device.provider);
    }

    PMstop();
}

IOReturn VoodooI2CController::setPowerState(unsigned long whichState, IOService* whatDevice) {
    return kIOPMAckImplied;
}

bool VoodooI2CController::start(IOService* provider) {
    if (!super::start(provider)) {
        goto exit;
    }

    physical_device.provider = provider;
    physical_device.name = getMatchedName(physical_device.provider);

    PMinit();
    physical_device.provider->joinPMtree(this);
    registerPowerDriver(this, VoodooI2CIOPMPowerStates, kVoodooI2CIOPMNumberPowerStates);

    IOLog("%s::%s Starting I2C controller\n", getName(), physical_device.name);

    physical_device.provider->retain();
    if (!physical_device.provider->open(this)) {
        IOLog("%s::%s Could not open provider\n", getName(), physical_device.name);
    }

    provider->setProperty("VoodooI2CServices Supported", kOSBooleanTrue);
    provider->setProperty("isI2CController", kOSBooleanTrue);

    return true;

exit:
    releaseResources();
    return false;
}

void VoodooI2CController::stop(IOService* provider) {
    releaseResources();

    super::stop(provider);
}

void VoodooI2CController::writeRegister(UInt32 value, int offset) {
    if (physical_device.mmap != 0) {
        IOVirtualAddress address = physical_device.mmap->getVirtualAddress();
        if (address != 0)
            *(volatile UInt32 *)(address + offset) = value;
        else
            TryLog("%s::%s writeRegister at 0x%x failed to get a virtual address\n", getName(), physical_device.name, offset);
    } else {
        TryLog("%s::%s writeRegister at 0x%x failed since mamory was not mapped\n", getName(), physical_device.name, offset);
    }
}
