//
//  VoodooHIDWrapper.cpp
//  VoodooI2C
//
//  Created by Christopher Luu on 10/7/15.
//  Copyright © 2015 Alexandre Daoud. All rights reserved.
//

#include "VoodooHIDWrapper.h"
#include "VoodooI2CHIDDevice.h"

OSDefineMetaClassAndStructors(VoodooHIDWrapper, IOHIDDevice)

static VoodooI2CHIDDevice* GetOwner(const IOService *us)
{
    IOService *prov = us->getProvider();
    
    if (prov == NULL)
        return NULL;
    return OSDynamicCast(VoodooI2CHIDDevice, prov);
}

bool VoodooHIDWrapper::start(IOService *provider) {
    if (OSDynamicCast(VoodooI2CHIDDevice, provider) == NULL)
        return false;

    setProperty("HIDDefaultBehavior", OSString::withCString("Mouse"));
    return IOHIDDevice::start(provider);
}

IOReturn VoodooHIDWrapper::setProperties(OSObject *properties) {
    return kIOReturnUnsupported;
}

IOReturn VoodooHIDWrapper::newReportDescriptor(IOMemoryDescriptor **descriptor) const {
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, GetOwner(this)->hid_device->hdesc.wReportDescLength);

    if (buffer == NULL) return kIOReturnNoResources;
    GetOwner(this)->write_report_descriptor_to_buffer(buffer);
    *descriptor = buffer;
    return kIOReturnSuccess;
}

IOReturn VoodooHIDWrapper::setReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    return kIOReturnUnsupported;
}

IOReturn VoodooHIDWrapper::getReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    return kIOReturnUnsupported;
}

IOReturn VoodooHIDWrapper::handleReport(
                                        IOMemoryDescriptor * report,
                                        IOHIDReportType      reportType,
                                        IOOptionBits         options  ) {
    return IOHIDDevice::handleReport(report, reportType, options);
}

OSString* VoodooHIDWrapper::newManufacturerString() const {
    return OSString::withCString("Apple");
}

OSNumber* VoodooHIDWrapper::newPrimaryUsageNumber() const {
    return OSNumber::withNumber(kHIDUsage_GD_Mouse, 32);
}

OSNumber* VoodooHIDWrapper::newPrimaryUsagePageNumber() const {
    return OSNumber::withNumber(kHIDPage_GenericDesktop, 32);
}

OSNumber* VoodooHIDWrapper::newProductIDNumber() const {
    return OSNumber::withNumber(GetOwner(this)->hid_device->hdesc.wProductID, 16);
}

OSString* VoodooHIDWrapper::newProductString() const {
    return OSString::withCString("I2C HID Device");
}

OSString* VoodooHIDWrapper::newSerialNumberString() const {
    return OSString::withCString("1234");
}

OSString* VoodooHIDWrapper::newTransportString() const {
    return OSString::withCString("I2C");
}

OSNumber* VoodooHIDWrapper::newVendorIDNumber() const {
    return OSNumber::withNumber(GetOwner(this)->hid_device->hdesc.wVendorID, 16);
}

OSNumber* VoodooHIDWrapper::newLocationIDNumber() const {
    return OSNumber::withNumber(123, 32);
}
