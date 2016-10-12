//
//  VoodooCSGestureHIDWrapper.cpp
//  VoodooI2C
//
//  Created by Christopher Luu on 10/7/15.
//  Copyright © 2015 Alexandre Daoud. All rights reserved.
//

#include "VoodooCSGestureHIDWrapper.h"
#include "csgesture.h"

OSDefineMetaClassAndStructors(VoodooCSGestureHIDWrapper, IOHIDDevice)

bool VoodooCSGestureHIDWrapper::start(IOService *provider) {
    if (!IOHIDDevice::start(provider))
        return false;
    setProperty("HIDDefaultBehavior", OSString::withCString("Trackpad"));
    return true;
}

IOReturn VoodooCSGestureHIDWrapper::setProperties(OSObject *properties) {
    return kIOReturnUnsupported;
}

IOReturn VoodooCSGestureHIDWrapper::newReportDescriptor(IOMemoryDescriptor **descriptor) const {    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, gestureEngine->reportDescriptorLength());

    if (buffer == NULL) return kIOReturnNoResources;
    gestureEngine->write_report_descriptor_to_buffer(buffer);
    *descriptor = buffer;
    return kIOReturnSuccess;
}

IOReturn VoodooCSGestureHIDWrapper::setReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    return kIOReturnUnsupported;
}

IOReturn VoodooCSGestureHIDWrapper::getReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    if (reportType == kIOHIDReportTypeOutput){
        gestureEngine->write_report_to_buffer(report);
        return kIOReturnSuccess;
    }
    return kIOReturnUnsupported;
}

OSString* VoodooCSGestureHIDWrapper::newManufacturerString() const {
    return OSString::withCString("CSGesture");
}

OSNumber* VoodooCSGestureHIDWrapper::newPrimaryUsageNumber() const {
    return OSNumber::withNumber(kHIDUsage_GD_Mouse, 32);
}

OSNumber* VoodooCSGestureHIDWrapper::newPrimaryUsagePageNumber() const {
    return OSNumber::withNumber(kHIDPage_GenericDesktop, 32);
}

OSNumber* VoodooCSGestureHIDWrapper::newProductIDNumber() const {
    return OSNumber::withNumber(gestureEngine->productID, 32);
}

OSString* VoodooCSGestureHIDWrapper::newProductString() const {
    return OSString::withCString("Trackpad");
}

OSString* VoodooCSGestureHIDWrapper::newSerialNumberString() const {
    return OSString::withCString("1234");
}

OSString* VoodooCSGestureHIDWrapper::newTransportString() const {
    return OSString::withCString("I2C");
}

OSNumber* VoodooCSGestureHIDWrapper::newVendorIDNumber() const {
    return OSNumber::withNumber(gestureEngine->vendorID, 16);
}

OSNumber* VoodooCSGestureHIDWrapper::newLocationIDNumber() const {
    return OSNumber::withNumber(123, 32);
}
