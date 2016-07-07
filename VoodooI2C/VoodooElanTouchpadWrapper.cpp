//
//  VoodooElanTouchpadWrapper.cpp
//  VoodooI2C
//
//  Created by Christopher Luu on 10/7/15.
//  Copyright © 2015 Alexandre Daoud. All rights reserved.
//

#include "VoodooElanTouchpadWrapper.h"
#include "VoodooElanTouchpadDevice.h"

OSDefineMetaClassAndStructors(VoodooElanTouchpadWrapper, IOHIDDevice)

static VoodooI2CElanTouchpadDevice* GetOwner(const IOService *us)
{
    IOService *prov = us->getProvider();
    
    if (prov == NULL)
        return NULL;
    return OSDynamicCast(VoodooI2CElanTouchpadDevice, prov);
}

bool VoodooElanTouchpadWrapper::start(IOService *provider) {
    if (OSDynamicCast(VoodooI2CElanTouchpadDevice, provider) == NULL)
        return false;

    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    setProperty("HIDDefaultBehavior", OSString::withCString("Trackpad"));
    return IOHIDDevice::start(provider);
}

IOReturn VoodooElanTouchpadWrapper::setProperties(OSObject *properties) {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return kIOReturnUnsupported;
}

IOReturn VoodooElanTouchpadWrapper::newReportDescriptor(IOMemoryDescriptor **descriptor) const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, GetOwner(this)->reportDescriptorLength());

    if (buffer == NULL) return kIOReturnNoResources;
    GetOwner(this)->write_report_descriptor_to_buffer(buffer);
    *descriptor = buffer;
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return kIOReturnSuccess;
}

IOReturn VoodooElanTouchpadWrapper::setReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return kIOReturnUnsupported;
}

IOReturn VoodooElanTouchpadWrapper::getReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    if (reportType == kIOHIDReportTypeOutput){
        GetOwner(this)->write_report_to_buffer(report);
        return kIOReturnSuccess;
    }
    return kIOReturnUnsupported;
}

/*IOReturn VoodooElanTouchpadWrapper::handleReport(
                                        IOMemoryDescriptor * report,
                                        IOHIDReportType      reportType,
                                        IOOptionBits         options  ) {
    return IOHIDDevice::handleReport(report, reportType, options);
}*/

OSString* VoodooElanTouchpadWrapper::newManufacturerString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("Cypress");
}

OSNumber* VoodooElanTouchpadWrapper::newPrimaryUsageNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(kHIDUsage_GD_Mouse, 32);
}

OSNumber* VoodooElanTouchpadWrapper::newPrimaryUsagePageNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(kHIDPage_GenericDesktop, 32);
}

OSNumber* VoodooElanTouchpadWrapper::newProductIDNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(GetOwner(this)->productID(), 32);
}

OSString* VoodooElanTouchpadWrapper::newProductString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("Gen3 Trackpad");
}

OSString* VoodooElanTouchpadWrapper::newSerialNumberString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("1234");
}

OSString* VoodooElanTouchpadWrapper::newTransportString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("I2C");
}

OSNumber* VoodooElanTouchpadWrapper::newVendorIDNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(GetOwner(this)->vendorID(), 16);
}

OSNumber* VoodooElanTouchpadWrapper::newLocationIDNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(123, 32);
}
