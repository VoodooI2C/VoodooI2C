//
//  VoodooI2CDeviceNub.cpp
//  VoodooI2C
//
//  Created by Alexandre on 07/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "../VoodooI2CController/VoodooI2CControllerDriver.hpp"
#include "VoodooI2CDeviceNub.hpp"
#include "VoodooI2CACPICRSParser.hpp"

#define super IOService
OSDefineMetaClassAndStructors(VoodooI2CDeviceNub, IOService);

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

    controller = OSDynamicCast(VoodooI2CControllerDriver, provider);

    if (!controller) {
        IOLog("%s::%s Could not get controller\n", controller_name, child->getName());
        return false;
    }

    controller->retain();

    if (getDeviceResources() != kIOReturnSuccess) {
        IOLog("%s::%s Could not get device resources\n", controller_name, child->getName());
        return false;
    }

    if (has_gpio_interrupts) {
        gpio_controller = getGPIOController();

        if (!gpio_controller) {
            IOLog("%s::%s Could not find GPIO controller, exiting", controller_name, child->getName());
            return false;
        }

        // Give the GPIO controller some time to load

        IOSleep(500);
        gpio_controller->retain();
    }

    setName(child->getName());

    return true;
}

IOReturn VoodooI2CDeviceNub::disableInterrupt(int source) {
    if (has_gpio_interrupts) {
        return gpio_controller->disableInterrupt(gpio_pin);
    } else {
        return acpi_device->disableInterrupt(source);
    }
}

IOReturn VoodooI2CDeviceNub::enableInterrupt(int source) {
    if (has_gpio_interrupts) {
        return gpio_controller->enableInterrupt(gpio_pin);
    } else {
        return acpi_device->enableInterrupt(source);
    }
}

IOReturn VoodooI2CDeviceNub::getDeviceResources() {
    OSObject *result = NULL;
    acpi_device->evaluateObject("_CRS", &result);

    OSData *data = OSDynamicCast(OSData, result);
    if (!data) {
        IOLog("%s::%s Could not find or evaluate _CRS method", controller_name, getName());
        return kIOReturnNotFound;
    }

    uint8_t const* crs = reinterpret_cast<uint8_t const*>(data->getBytesNoCopy());
    VoodooI2CACPICRSParser crs_parser;
    crs_parser.parseACPICRS(crs, 0, data->getLength());

    if (crs_parser.found_i2c) {
        use_10bit_addressing = crs_parser.i2c_info.address_mode_10Bit;
        setProperty("addrWidth", use_10bit_addressing ? 10 : 7, 8);

        i2c_address = crs_parser.i2c_info.address;
        setProperty("i2cAddress", i2c_address, 16);

        setProperty("sclHz", crs_parser.i2c_info.bus_speed, 32);
    } else {
        IOLog("%s::%s Could not find an I2C Serial Bus declaration\n", controller_name, getName());
        return kIOReturnNotFound;
    }

    if (crs_parser.found_gpio_interrupts) {
        setProperty("gpioPin", crs_parser.gpio_interrupts.pin_number, 16);
        setProperty("gpioIRQ", crs_parser.gpio_interrupts.irq_type, 16);

        has_gpio_interrupts = true;
        gpio_pin = crs_parser.gpio_interrupts.pin_number;
        gpio_irq = crs_parser.gpio_interrupts.irq_type;
    } else {
        has_gpio_interrupts = false;
    }

    if (!has_gpio_interrupts) {
        OSArray* interrupt_array = OSDynamicCast(OSArray, acpi_device->getProperty(gIOInterruptSpecifiersKey));
        if (!interrupt_array) {
            IOLog("%s::%s Warning: Could not find any APIC nor GPIO interrupts; if your chosen satellite implements polling then %s will run in polling mode.\n", controller_name, getName(), getName());
            goto exit;
        }

        OSData* interrupt_data = OSDynamicCast(OSData, interrupt_array->getObject(0));

        if (!interrupt_data) {
            IOLog("%s::%s Warning: Could not find any APIC nor GPIO interrupts; if your chosen satellite implements polling then %s will run in polling mode.\n", controller_name, getName(), getName());
            goto exit;
        }

        const UInt16* interrupt_pin = reinterpret_cast<const UInt16*>(interrupt_data->getBytesNoCopy(0, 1));

        if (!interrupt_pin) {
            IOLog("%s::%s Warning: Could not find any APIC nor GPIO interrupts; if your chosen satellite implements polling then %s will run in polling mode.\n", controller_name, getName(), getName());
            goto exit;
        }

        if (*interrupt_pin > 0x2f)
            IOLog("%s::%s Warning: Incompatible APIC interrupt pin (0x%x > 0x2f) and no GPIO interrupts found; if your chosen satellite implements polling then %s will run in polling mode.\n", controller_name, getName(), *interrupt_pin, getName());
    }
exit:
    data->release();
    return kIOReturnSuccess;
}

VoodooGPIO* VoodooI2CDeviceNub::getGPIOController() {
    VoodooGPIO* gpio_controller = NULL;

    // Wait for GPIO controller, up to 1 second
    OSDictionary* name_match = IOService::serviceMatching("VoodooGPIO");

    IOService* matched = waitForMatchingService(name_match, 1000000000);
    gpio_controller = OSDynamicCast(VoodooGPIO, matched);

    if (gpio_controller != NULL) {
        IOLog("%s::Got GPIO Controller! %s\n", getName(), gpio_controller->getName());
    }
    name_match->release();
    OSSafeReleaseNULL(matched);

    return gpio_controller;
}

IOReturn VoodooI2CDeviceNub::getInterruptType(int source, int* interrupt_type) {
    if (has_gpio_interrupts) {
        return gpio_controller->getInterruptType(gpio_pin, interrupt_type);
    } else {
        return acpi_device->getInterruptType(source, interrupt_type);
    }
}

IOWorkLoop* VoodooI2CDeviceNub::getWorkLoop() {
    // Do we have a work loop already?, if so return it NOW.
    if ((vm_address_t) work_loop >> 1)
        return work_loop;

    if (OSCompareAndSwap(0, 1, reinterpret_cast<IOWorkLoop*>(&work_loop))) {
        // Construct the workloop and set the cntrlSync variable
        // to whatever the result is and return
        work_loop = IOWorkLoop::workLoop();
    } else {
        while (reinterpret_cast<IOWorkLoop*>(work_loop) == reinterpret_cast<IOWorkLoop*>(1)) {
            // Spin around the cntrlSync variable until the
            // initialization finishes.
            thread_block(0);
        }
    }

    return work_loop;
}

IOReturn VoodooI2CDeviceNub::readI2C(UInt8* values, UInt16 length) {
    return command_gate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooI2CDeviceNub::readI2CGated), values, &length);
}

IOReturn VoodooI2CDeviceNub::readI2CGated(UInt8* values, UInt16* length) {
    UInt16 flags = I2C_M_RD;

    if (use_10bit_addressing)
        flags |= I2C_M_TEN;
    VoodooI2CControllerBusMessage msgs[] = {
        {
            .address = i2c_address,
            .buffer = values,
            .flags = flags,
            .length = *length,
        },
    };

    return controller->transferI2C(msgs, 1);
}

IOReturn VoodooI2CDeviceNub::registerInterrupt(int source, OSObject *target, IOInterruptAction handler, void *refcon) {
    if (has_gpio_interrupts) {
        gpio_controller->setInterruptTypeForPin(gpio_pin, gpio_irq);
        return gpio_controller->registerInterrupt(gpio_pin, target, handler, refcon);
    } else {
        return acpi_device->registerInterrupt(source, target, handler, refcon);
    }
    return kIOReturnSuccess;
}

void VoodooI2CDeviceNub::releaseResources() {
    if (command_gate) {
        work_loop->removeEventSource(command_gate);
        OSSafeReleaseNULL(command_gate);
    }

    OSSafeReleaseNULL(work_loop);
}

bool VoodooI2CDeviceNub::start(IOService* provider) {
    if (!super::start(provider))
        return false;

    work_loop = getWorkLoop();

    if (!work_loop) {
        IOLog("%s Could not get work loop\n", getName());
        goto exit;
    }

    work_loop->retain();

    command_gate = IOCommandGate::commandGate(this);
    if (!command_gate || (work_loop->addEventSource(command_gate) != kIOReturnSuccess)) {
        IOLog("%s Could not open command gate\n", getName());
        goto exit;
    }

    setProperty("IOName", reinterpret_cast<const char*>(OSDynamicCast(OSData, getProperty("name"))->getBytesNoCopy()));

    registerService();

    setProperty("VoodooI2CServices Supported", kOSBooleanTrue);

    return true;
exit:
    releaseResources();
    return false;
}

void VoodooI2CDeviceNub::stop(IOService* provider) {
    releaseResources();
    OSSafeReleaseNULL(gpio_controller);
    OSSafeReleaseNULL(controller);
    super::stop(provider);
}

IOReturn VoodooI2CDeviceNub::unregisterInterrupt(int source) {
    if (has_gpio_interrupts) {
        return gpio_controller->unregisterInterrupt(gpio_pin);
    } else {
        return acpi_device->unregisterInterrupt(source);
    }
}

IOReturn VoodooI2CDeviceNub::writeI2C(UInt8 *values, UInt16 length) {
    return command_gate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooI2CDeviceNub::writeI2CGated), values, &length);
}

IOReturn VoodooI2CDeviceNub::writeI2CGated(UInt8* values, UInt16* length) {
    UInt16 flags = 0;
    if (use_10bit_addressing)
        flags = I2C_M_TEN;

    VoodooI2CControllerBusMessage msgs[] = {
        {
            .address = i2c_address,
            .buffer = values,
            .flags = flags,
            .length = *length,
        },
    };
    return controller->transferI2C(msgs, 1);
}

IOReturn VoodooI2CDeviceNub::writeReadI2C(UInt8 *write_buffer, UInt16 write_length, UInt8 *read_buffer, UInt16 read_length) {
    return command_gate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooI2CDeviceNub::writeReadI2CGated), write_buffer, &write_length, read_buffer, &read_length);
}
IOReturn VoodooI2CDeviceNub::writeReadI2CGated(UInt8* write_buffer, UInt16* write_length, UInt8* read_buffer, UInt16* read_length) {
    UInt16 read_flags = I2C_M_RD;
    if (use_10bit_addressing)
        read_flags |= I2C_M_TEN;

    UInt16 write_flags = 0;

    if (use_10bit_addressing)
        write_flags = I2C_M_TEN;
    VoodooI2CControllerBusMessage msgs[] = {
        {
            .address = i2c_address,
            .buffer = write_buffer,
            .flags = write_flags,
            .length = *write_length,
        },
        {
            .address = i2c_address,
            .buffer = read_buffer,
            .flags = read_flags,
            .length = *read_length,
        }
    };
    return controller->transferI2C(msgs, 2);
}
