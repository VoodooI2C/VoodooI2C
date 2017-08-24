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

    controller_name = provider->getName();
    setProperty("acpi-device", child);
    acpi_device = OSDynamicCast(IOACPIPlatformDevice, child);

    if (!acpi_device) {
        IOLog("%s::%s Could not get ACPI device\n", controller_name, child->getName());
        return false;
    }

    if (getDeviceResources() != kIOReturnSuccess) {
        IOLog("%s::%s Could not get device resources\n", controller_name, child->getName());
        return false;
    }

    if (this->has_gpio_interrupts) {
        this->gpio_controller = getGPIOController();
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

IOReturn VoodooI2CDeviceNub::getDeviceResources() {
    OSObject *result = NULL;
    acpi_device->evaluateObject("_CRS", &result);

    OSData *data = OSDynamicCast(OSData, result);
    if (!data) {
        IOLog("%s::%s Could not find or evaluate _CRS method", controller_name, getName());
        return kIOReturnNotFound;
    }

    const uint8_t *crs = reinterpret_cast<const uint8_t*>(data->getBytesNoCopy());
    VoodooI2CACPICRSParser crs_parser;
    crs_parser.parseACPICRS(crs, 0, data->getLength());

    if (crs_parser.found_i2c) {
        setProperty("addrWidth", OSNumber::withNumber(crs_parser.i2c_info.address_mode_10Bit ? 10 : 7, 8));
        setProperty("i2cAddress", OSNumber::withNumber(crs_parser.i2c_info.address, 16));
        setProperty("sclHz", OSNumber::withNumber(crs_parser.i2c_info.bus_speed, 32));
    } else {
        IOLog("%s::%s Could not find an I2C Serial Bus declaration\n", controller_name, getName());
        return kIOReturnNotFound;
    }

    if (crs_parser.found_gpio_interrupts) {
        setProperty("gpioPin", OSNumber::withNumber(crs_parser.gpio_interrupts.pin_number, 16));
        setProperty("gpioIRQ", OSNumber::withNumber(crs_parser.gpio_interrupts.irq_type, 16));

        this->has_gpio_interrupts = true;
        this->gpio_pin = crs_parser.gpio_interrupts.pin_number;
        this->gpio_irq = crs_parser.gpio_interrupts.irq_type;
    } else {
        this->has_gpio_interrupts = false;
    }

    data->release();
    return kIOReturnSuccess;
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
    if (this->gpio_controller) {
        this->gpio_controller->release();
        this->gpio_controller = NULL;
    }

    super::stop(provider);
}

/**
 Get the GPIO controller instance
 */
VoodooGPIO *VoodooI2CDeviceNub::getGPIOController() {
    VoodooGPIO* gpio_controller = NULL;

    OSDictionary *match = serviceMatching("VoodooGPIO");
    OSIterator *iterator = getMatchingServices(match);
    if (iterator) {
        gpio_controller = OSDynamicCast(VoodooGPIO, iterator->getNextObject());

        if (gpio_controller != NULL) {
            IOLog("%s::Got GPIO Controller! %s\n", getName(), gpio_controller->getName());
        }

        gpio_controller->retain();

        iterator->release();
    }

    return gpio_controller;
}

/*
Interrupt functions
*/

IOReturn VoodooI2CDeviceNub::disableInterrupt(int source) {
    if (this->has_gpio_interrupts) {
        return gpio_controller->disableInterrupt(this->gpio_pin);
    } else {
        return acpi_device->disableInterrupt(source);
    }
}

IOReturn VoodooI2CDeviceNub::enableInterrupt(int source) {
    if (this->has_gpio_interrupts) {
         return gpio_controller->enableInterrupt(this->gpio_pin);
    } else {
        return acpi_device->enableInterrupt(source);
    }
}

IOReturn VoodooI2CDeviceNub::getInterruptType(int source, int* interrupt_type) {
    if (this->has_gpio_interrupts) {
        return gpio_controller->getInterruptType(this->gpio_pin, interrupt_type);
    } else {
        return acpi_device->getInterruptType(source, interrupt_type);
    }
}

IOReturn VoodooI2CDeviceNub::registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refcon) {
    if (this->has_gpio_interrupts) {
        this->gpio_controller->setInterruptTypeForPin(this->gpio_pin, this->gpio_irq);
        return gpio_controller->registerInterrupt(this->gpio_pin, target, handler, refcon);
    } else {
        return acpi_device->registerInterrupt(source, target, handler, refcon);
    }
}

IOReturn VoodooI2CDeviceNub::unregisterInterrupt(int source) {
    if (this->has_gpio_interrupts) {
        return this->gpio_controller->unregisterInterrupt(this->gpio_pin);
    } else {
        return acpi_device->unregisterInterrupt(source);
    }
}
