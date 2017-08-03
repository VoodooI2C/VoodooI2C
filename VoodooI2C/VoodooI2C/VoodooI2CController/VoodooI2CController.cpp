//
//  VoodooI2CController.cpp
//
//  Created by Alexandre on 31/07/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CController.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CController, IOService);

/**
 Frees VoodooI2CController class - releases objects instantiated in `init`
 */

void VoodooI2CController::free() {
    IOFree(physical_device, sizeof(physical_device));

    super::free();
}

/**
 Initialises VoodooI2CController class

 @param properties OSDictionary* representing the matched personality

 @return returns true on successful initialisation, else returns false
 */

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

/**
 Handles an interrupt when the controller asserts its interrupt line

 @param owner    OSOBject* that owns this interrupt
 @param src      IOInterruptEventSource*
 @param intCount int representing the index of the interrupt in the provider
 */

void VoodooI2CController::interruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount) {
}

/**
 Starts the physical I2C controller

 @param provider IOService* representing the matched entry in the IORegistry

 @return returns true on succesful start, else returns false
 */

IOReturn VoodooI2CController::mapMemory() {
    if (physical_device->provider->getDeviceMemoryCount() == 0) {
        return kIOReturnDeviceError;
    } else {
        physical_device->mmap = physical_device->provider->mapDeviceMemoryWithIndex(0);
        if (!physical_device->mmap) return kIOReturnDeviceError;
        return kIOReturnSuccess;
    }
}

/**
 Implemented to beat Apple's own LPSS kexts
 
 @param provider IOService* representing the matched entry in the IORegistry
 @param score    Probe score as specified in the matched personality
 
 @return returns a pointer to this instance of VoodooI2CController
 */

VoodooI2CController* VoodooI2CController::probe(IOService* provider, SInt32* score) {
    if (!super::probe(provider, score)) {
        if (debug_logging)
            IOLog("%s::%s super::probe failed", getName(), getMatchedName(provider));
        return NULL;
    }

    return this;
}

/**
 Reads the register of the controller at the offset

 @param offset

 @return returns the value of the register
 */
UInt32 VoodooI2CController::readRegister(int offset) {
    return *(const volatile UInt32 *)(physical_device->mmap->getVirtualAddress() + offset);
}

/**
 Called by the system's power manager to set power states

 @param whichState either kIOPMPowerOff or kIOPMPowerOn
 @param whatDevice Power management policy maker

 @return returns kIOPMAckImplied if power state has been set else maximum number of milliseconds needed for the device to be in the correct state
 */
IOReturn VoodooI2CController::setPowerState(unsigned long whichState, IOService * whatDevice) {
    return kIOPMAckImplied;
}

/**
 Starts the physical I2C controller
 
 @param provider IOService* representing the matched entry in the IORegistry
 
 @return returns true on succesful start, else returns false
 */

bool VoodooI2CController::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }

    physical_device->provider = provider;
    physical_device->name = getMatchedName(physical_device->provider);

    PMinit();
    physical_device->provider->joinPMtree(this);
    registerPowerDriver(this, VoodooI2CControllerPowerStates, kVoodooI2CIOPMNumberPowerStates);

    IOLog("%s::%s Starting I2C controller\n", getName(), physical_device->name);

    physical_device->provider->retain();
    if (!physical_device->provider->open(this))
        return NULL;

    physical_device->work_loop = reinterpret_cast<IOWorkLoop*>(getWorkLoop());
    if (!physical_device->work_loop) {
        IOLog("%s::%s Failed to get work loop\n", getName(), physical_device->name);
        return NULL;
    }

    physical_device->work_loop->retain();

    physical_device->interrupt_source =
    IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CController::interruptOccured), physical_device->provider);

    if (!physical_device->interrupt_source || physical_device->work_loop->addEventSource(physical_device->interrupt_source) != kIOReturnSuccess) {
        IOLog("%s::%s::Could not add interrupt source to work loop\n", getName(), physical_device->name);
        return NULL;
    }

    physical_device->interrupt_source->enable();

    physical_device->command_gate = IOCommandGate::commandGate(this);
    if (!physical_device->command_gate || (physical_device->work_loop->addEventSource(physical_device->command_gate) != kIOReturnSuccess)) {
        IOLog("%s::%s::Failed to open command gate\n", getName(), physical_device->name);
        return false;
    }

    return true;
}

/**
 Stops the physical I2C controller and undoes the effects of `start` and `probe`

 @param provider IOService* representing the matched entry in the IORegistry
 */

void VoodooI2CController::stop(IOService* provider) {
    if (physical_device) {
        if (physical_device->mmap) {
            physical_device->mmap->release();
            physical_device->mmap = NULL;
        }

        if (physical_device->command_gate) {
            physical_device->work_loop->removeEventSource(physical_device->command_gate);
            physical_device->command_gate->release();
            physical_device->command_gate = NULL;
        }

        if (physical_device->interrupt_source) {
            physical_device->interrupt_source->disable();
            physical_device->work_loop->removeEventSource(physical_device->interrupt_source);
            physical_device->interrupt_source->release();
            physical_device->interrupt_source = NULL;
        }
        if (physical_device->work_loop)
            OSSafeReleaseNULL(physical_device->work_loop);

       // if (physical_device->pci_device)
        //    physical_device->pci_device = NULL;

        physical_device->provider->close(this);
        OSSafeReleaseNULL(physical_device->provider);
    }

    PMstop();

    super::stop(provider);
}

/**
 Writes the `value` into the controller's register at the `offset`

 @param value
 @param offset
 */

void VoodooI2CController::writeRegister(UInt32 value, int offset) {
    *(volatile UInt32 *)(physical_device->mmap->getVirtualAddress() + offset) = value;
}
