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
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    setProperty("HIDDefaultBehavior", OSString::withCString("Trackpad"));
    return IOHIDDevice::start(provider);
}

IOReturn VoodooCSGestureHIDWrapper::setProperties(OSObject *properties) {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return kIOReturnUnsupported;
}

IOReturn VoodooCSGestureHIDWrapper::newReportDescriptor(IOMemoryDescriptor **descriptor) const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, gestureEngine->reportDescriptorLength());

    if (buffer == NULL) return kIOReturnNoResources;
    gestureEngine->write_report_descriptor_to_buffer(buffer);
    *descriptor = buffer;
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return kIOReturnSuccess;
}

IOReturn VoodooCSGestureHIDWrapper::setReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return kIOReturnUnsupported;
}

IOReturn VoodooCSGestureHIDWrapper::getReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    if (reportType == kIOHIDReportTypeOutput){
        gestureEngine->write_report_to_buffer(report);
        return kIOReturnSuccess;
    }
    return kIOReturnUnsupported;
}

/*IOReturn VoodooCyapaMouseWrapper::handleReport(
                                        IOMemoryDescriptor * report,
                                        IOHIDReportType      reportType,
                                        IOOptionBits         options  ) {
    return IOHIDDevice::handleReport(report, reportType, options);
}*/

OSString* VoodooCSGestureHIDWrapper::newManufacturerString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("CSGesture");
}

OSNumber* VoodooCSGestureHIDWrapper::newPrimaryUsageNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(kHIDUsage_GD_Mouse, 32);
}

OSNumber* VoodooCSGestureHIDWrapper::newPrimaryUsagePageNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(kHIDPage_GenericDesktop, 32);
}

OSNumber* VoodooCSGestureHIDWrapper::newProductIDNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(gestureEngine->productID, 32);
}

OSString* VoodooCSGestureHIDWrapper::newProductString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("Trackpad");
}

OSString* VoodooCSGestureHIDWrapper::newSerialNumberString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("1234");
}

OSString* VoodooCSGestureHIDWrapper::newTransportString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("I2C");
}

OSNumber* VoodooCSGestureHIDWrapper::newVendorIDNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(gestureEngine->vendorID, 16);
}

OSNumber* VoodooCSGestureHIDWrapper::newLocationIDNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(123, 32);
}
