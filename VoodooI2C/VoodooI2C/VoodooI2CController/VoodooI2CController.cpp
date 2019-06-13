//
//  VoodooI2CController.cpp
//
//  Created by Alexandre on 31/07/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CController.hpp"
#include "VoodooI2CControllerNub.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CController, IOService);

void VoodooI2CController::free() {
    IOFree(physical_device, sizeof(VoodooI2CControllerPhysicalDevice));

    super::free();
}

bool VoodooI2CController::init(OSDictionary* properties) {
    if (!super::init(properties)) {
        if (debug_logging)
            IOLog("%s super::init failed\n", getName());
        return false;
    }

    physical_device = reinterpret_cast<VoodooI2CControllerPhysicalDevice*>(IOMalloc(sizeof(VoodooI2CControllerPhysicalDevice)));
    physical_device->awake = true;

    return true;
}

IOReturn VoodooI2CController::mapMemory() {
    if (physical_device->provider->getDeviceMemoryCount() == 0) {
        return kIOReturnDeviceError;
    } else {
        physical_device->mmap = physical_device->provider->mapDeviceMemoryWithIndex(0);
        if (!physical_device->mmap) return kIOReturnDeviceError;
        return kIOReturnSuccess;
    }
}

VoodooI2CController* VoodooI2CController::probe(IOService* provider, SInt32* score) {
    if (!super::probe(provider, score)) {
        if (debug_logging)
            IOLog("%s::%s super::probe failed", getName(), getMatchedName(provider));
        return NULL;
    }

    return this;
}

IOReturn VoodooI2CController::publishNub() {
    IOLog("%s::%s Publishing nub\n", getName(), physical_device->name);
    bool was_attached = false;
    bool was_started = false;
    nub = new VoodooI2CControllerNub;

    if (!nub || !nub->init()) {
        IOLog("%s::%s Could not initialise nub", getName(), physical_device->name);
        goto exit;
    }
   
    if (!nub->attach(this)) {
        IOLog("%s::%s Could not attach nub", getName(), physical_device->name);
        goto exit;
    }
    was_attached = true;

    if (!nub->start(this)) {
        IOLog("%s::%s Could not start nub", getName(), physical_device->name);
        goto exit;
    }
    was_started = true;

    setProperty("VoodooI2CServices Supported", kOSBooleanTrue);

    return kIOReturnSuccess;

exit:
    if (nub) {
        if(was_started) {
             nub->stop(this);
        }

        if(was_attached) {
            nub->detach(this);
        }
       
        OSSafeReleaseNULL(nub);
    }

    return kIOReturnError;
}

UInt32 VoodooI2CController::readRegister(int offset) {
    return *(const volatile UInt32 *)(physical_device->mmap->getVirtualAddress() + offset);
}

void VoodooI2CController::releaseResources() {
    if (nub) {
        nub->stop(this);
        nub->detach(this);
        OSSafeReleaseNULL(nub);
    }
    
    if (physical_device) {
        OSSafeReleaseNULL(physical_device->mmap);

        if(physical_device->provider) {
            physical_device->provider->close(this);
        }
        
        OSSafeReleaseNULL(physical_device->provider);
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

    physical_device->provider = provider;
    physical_device->name = getMatchedName(physical_device->provider);

    PMinit();
    physical_device->provider->joinPMtree(this);
    registerPowerDriver(this, VoodooI2CIOPMPowerStates, kVoodooI2CIOPMNumberPowerStates);

    IOLog("%s::%s Starting I2C controller\n", getName(), physical_device->name);

    physical_device->provider->retain();
    if (!physical_device->provider->open(this)) {
        IOLog("%s::%s Could not open provider\n", getName(), physical_device->name);
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
    *(volatile UInt32 *)(physical_device->mmap->getVirtualAddress() + offset) = value;
}
