//
//  VoodooI2CMT2ActuatorDevice.cpp
//  VoodooI2C
//
//  Created by CoolStar on 12/19/18.
//  Copyright © 2018 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CMT2ActuatorDevice.hpp"

#define super IOHIDDevice
OSDefineMetaClassAndStructors(VoodooI2CMT2ActuatorDevice, IOHIDDevice);

unsigned char actuator_report_descriptor[] = {0x06, 0x00, 0xff, 0x09, 0x0d, 0xa1, 0x01, 0x06, 0x00, 0xff, 0x09, 0x0d, 0x15, 0x00, 0x26, 0xff, 0x00, 0x75, 0x08, 0x85, 0x3f, 0x96, 0x0f, 0x00, 0x81, 0x02, 0x09, 0x0d, 0x85, 0x53, 0x96, 0x3f, 0x00, 0x91, 0x02, 0xc0};

IOReturn VoodooI2CMT2ActuatorDevice::setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options) {
    return kIOReturnSuccess;
}


IOReturn VoodooI2CMT2ActuatorDevice::newReportDescriptor(IOMemoryDescriptor** descriptor) const {
    IOBufferMemoryDescriptor* report_descriptor_buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, sizeof(actuator_report_descriptor));
    
    if (!report_descriptor_buffer) {
        IOLog("%s Could not allocate buffer for report descriptor\n", getName());
        return kIOReturnNoResources;
    }
    
    report_descriptor_buffer->writeBytes(0, actuator_report_descriptor, sizeof(actuator_report_descriptor));
    *descriptor = report_descriptor_buffer;
    
    return kIOReturnSuccess;
}

OSString* VoodooI2CMT2ActuatorDevice::newManufacturerString() const {
    return OSString::withCString("Apple Inc.");
}

OSNumber* VoodooI2CMT2ActuatorDevice::newPrimaryUsageNumber() const {
    return OSNumber::withNumber(0xd, 32);
}

OSNumber* VoodooI2CMT2ActuatorDevice::newPrimaryUsagePageNumber() const {
    return OSNumber::withNumber(0xff00, 32);
}

OSNumber* VoodooI2CMT2ActuatorDevice::newProductIDNumber() const {
    return OSNumber::withNumber(0x272, 32);
}

OSString* VoodooI2CMT2ActuatorDevice::newProductString() const {
    return OSString::withCString("Magic Trackpad 2");
}

OSString* VoodooI2CMT2ActuatorDevice::newSerialNumberString() const {
    return OSString::withCString("VoodooI2C Magic Trackpad 2 Simulator");
}

OSString* VoodooI2CMT2ActuatorDevice::newTransportString() const {
    return OSString::withCString("I2C");
}

OSNumber* VoodooI2CMT2ActuatorDevice::newVendorIDNumber() const {
    return OSNumber::withNumber(0x5ac, 16);
}

OSNumber* VoodooI2CMT2ActuatorDevice::newLocationIDNumber() const {
    return OSNumber::withNumber(0x14400000, 32);
}

OSNumber* VoodooI2CMT2ActuatorDevice::newVersionNumber() const {
    return OSNumber::withNumber(0x804, 32);
}
