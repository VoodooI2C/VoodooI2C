//
//  VoodooI2CDeviceNub.cpp
//  VoodooI2C
//
//  Created by Alexandre on 07/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CDeviceNub.hpp"
#include "VoodooI2CACPICRSParser.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CDeviceNub, IOService);

/**
 Attaches an IOService object to `this`
 
 @param provider IOService* representing the provider
 
 @return returns true on succesful attach, else returns false
 */

bool VoodooI2CDeviceNub::attach(IOService* provider, IOService* child) {
    if (!super::attach(provider))
        return false;

    setProperty("acpi-device", child);
    acpi_device = OSDynamicCast(IOACPIPlatformDevice, child);

    if (!get_device_resources())
        return false;
    
    if (this->hasGPIOInt){
        this->gpioController = getGPIOController();
    }

    setName(child->getName());

    return true;
}

/**
 Deattaches an IOService object from `this`
 
 @param provider IOService* representing the provider
 */

void VoodooI2CDeviceNub::detach(IOService* provider) {
    super::detach(provider);
}

/**
 Initialises class

 @param properties OSDictionary* representing the matched personality

 @return returns true on successful initialisation, else returns false
 */

bool VoodooI2CDeviceNub::init(OSDictionary* properties) {
    if (!super::init(properties))
        return false;

    return true;
}

/**
 Frees class - releases objects instantiated in `init`
 */

void VoodooI2CDeviceNub::free() {
    super::free();
}

bool VoodooI2CDeviceNub::get_device_resources() {
    OSObject *result = NULL;
    acpi_device->evaluateObject("_CRS", &result);

    OSData *data = OSDynamicCast(OSData, result);
    if (!data)
        return false;

    const uint8_t *crs = reinterpret_cast<const uint8_t*>(data->getBytesNoCopy());
    VoodooI2CACPICRSParser crsParser;
    crsParser.parse_acpi_crs(crs, 0, data->getLength());

    if (crsParser.foundI2C) {
        setProperty("addrWidth", OSNumber::withNumber(crsParser.i2cInfo.addressMode10Bit ? 10 : 7, 8));
        setProperty("i2cAddress", OSNumber::withNumber(crsParser.i2cInfo.address, 16));
        setProperty("sclHz", OSNumber::withNumber(crsParser.i2cInfo.busSpeed, 32));
    } else {
        return false;
    }

    if (crsParser.foundGPIOInt) {
        setProperty("gpioPin", OSNumber::withNumber(crsParser.gpioInt.pinNumber, 16));
        setProperty("gpioIRQ", OSNumber::withNumber(crsParser.gpioInt.irqType, 16));
        
        this->hasGPIOInt = true;
        this->gpioPin = crsParser.gpioInt.pinNumber;
        this->gpioIRQ = crsParser.gpioInt.irqType;
    } else {
        this->hasGPIOInt = false;
    }

    data->release();
    return true;
}

/**
 Starts the class
 
 @param provider IOService* representing the matched entry in the IORegistry
 
 @return returns true on succesful start, else returns false
 */

bool VoodooI2CDeviceNub::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    registerService();

    return true;
}

/**
 Stops the class and undoes the effects of `start` and `probe`

 @param provider IOService* representing the matched entry in the IORegistry
 */

void VoodooI2CDeviceNub::stop(IOService* provider) {
    if (this->gpioController){
        this->gpioController->release();
        this->gpioController = NULL;
    }
    
    super::stop(provider);
}

/**
 Get the GPIO controller instance
 */
VoodooGPIO *VoodooI2CDeviceNub::getGPIOController(){
    VoodooGPIO *gpioController = NULL;
    
    OSDictionary *match = serviceMatching("VoodooGPIO");
    OSIterator *iter = getMatchingServices(match);
    if (iter){
        gpioController = OSDynamicCast(VoodooGPIO, iter->getNextObject());
    
        if (gpioController != NULL){
            IOLog("%s::Got GPIO Controller! %s\n", getName(), gpioController->getName());
        }
    
        gpioController->retain();
    
        iter->release();
    }
    
    return gpioController;
}

/*
Interrupt functions
*/

IOReturn VoodooI2CDeviceNub::disableInterrupt(int source){
    if (this->hasGPIOInt){
        return gpioController->disableInterrupt(this->gpioPin);
    } else {
        return acpi_device->disableInterrupt(source);
    }
}

IOReturn VoodooI2CDeviceNub::enableInterrupt(int source){
    if (this->hasGPIOInt){
         return gpioController->enableInterrupt(this->gpioPin);
    } else {
        return acpi_device->enableInterrupt(source);
    }
}

IOReturn VoodooI2CDeviceNub::getInterruptType(int source, int *interruptType){
    if (this->hasGPIOInt){
        return gpioController->getInterruptType(this->gpioPin, interruptType);
    } else {
        return acpi_device->getInterruptType(source, interruptType);
    }
}

IOReturn VoodooI2CDeviceNub::registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refcon){
    if (this->hasGPIOInt){
        this->gpioController->setInterruptTypeForPin(this->gpioPin, this->gpioIRQ);
        return gpioController->registerInterrupt(this->gpioPin, target, handler, refcon);
    } else {
        return acpi_device->registerInterrupt(source, target, handler, refcon);
    }
}

IOReturn VoodooI2CDeviceNub::unregisterInterrupt(int source){
    if (this->hasGPIOInt){
        return this->gpioController->unregisterInterrupt(this->gpioPin);
    } else {
        return acpi_device->unregisterInterrupt(source);
    }
}
