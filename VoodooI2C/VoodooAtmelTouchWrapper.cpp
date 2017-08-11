//
//  VoodooAtmelTouchWrapper.cpp
//  VoodooI2C
//
//  Created by Christopher Luu on 10/7/15.
//  Copyright © 2015 Alexandre Daoud. All rights reserved.
//

#include "VoodooAtmelTouchWrapper.h"
#include "VoodooI2CAtmelMxtScreenDevice.h"

OSDefineMetaClassAndStructors(VoodooAtmelTouchWrapper, IOHIDDevice)

static VoodooI2CAtmelMxtScreenDevice* GetOwner(const IOService *us)
{
    IOService *prov = us->getProvider();
    
    if (prov == NULL)
        return NULL;
    return OSDynamicCast(VoodooI2CAtmelMxtScreenDevice, prov);
}

bool VoodooAtmelTouchWrapper::start(IOService *provider) {
    if (OSDynamicCast(VoodooI2CAtmelMxtScreenDevice, provider) == NULL)
        return false;
    if (!IOHIDDevice::start(provider))
        return false;

    setProperty("HIDDefaultBehavior", OSString::withCString("Trackpad"));
    return true;
}

IOReturn VoodooAtmelTouchWrapper::setProperties(OSObject *properties) {
    return kIOReturnUnsupported;
}

IOReturn VoodooAtmelTouchWrapper::newReportDescriptor(IOMemoryDescriptor **descriptor) const {
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, GetOwner(this)->reportDescriptorLength());

    if (buffer == NULL) return kIOReturnNoResources;
    GetOwner(this)->write_report_descriptor_to_buffer(buffer);
    *descriptor = buffer;
    return kIOReturnSuccess;
}

IOReturn VoodooAtmelTouchWrapper::setReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    return kIOReturnUnsupported;
}

IOReturn VoodooAtmelTouchWrapper::getReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    if (reportType == kIOHIDReportTypeOutput){
        GetOwner(this)->write_report_to_buffer(report);
        return kIOReturnSuccess;
    }
    return kIOReturnUnsupported;
}

/*IOReturn VoodooAtmelTouchWrapper::handleReport(
                                        IOMemoryDescriptor * report,
                                        IOHIDReportType      reportType,
                                        IOOptionBits         options  ) {
    return IOHIDDevice::handleReport(report, reportType, options);
}*/

OSString* VoodooAtmelTouchWrapper::newManufacturerString() const {
    return OSString::withCString("Atmel");
}

OSNumber* VoodooAtmelTouchWrapper::newPrimaryUsageNumber() const {
    return OSNumber::withNumber(kHIDUsage_Dig_TouchScreen, 32);
}

OSNumber* VoodooAtmelTouchWrapper::newPrimaryUsagePageNumber() const {
    return OSNumber::withNumber(kHIDPage_Digitizer, 32);
}

OSNumber* VoodooAtmelTouchWrapper::newProductIDNumber() const {
    return OSNumber::withNumber(GetOwner(this)->productID(), 32);
}

OSString* VoodooAtmelTouchWrapper::newProductString() const {
    return OSString::withCString("MaxTouch Touch Screen");
}

OSString* VoodooAtmelTouchWrapper::newSerialNumberString() const {
    return OSString::withCString("1234");
}

OSString* VoodooAtmelTouchWrapper::newTransportString() const {
    return OSString::withCString("I2C");
}

OSNumber* VoodooAtmelTouchWrapper::newVendorIDNumber() const {
    return OSNumber::withNumber(GetOwner(this)->vendorID(), 16);
}

OSNumber* VoodooAtmelTouchWrapper::newLocationIDNumber() const {
    return OSNumber::withNumber(123, 32);
}
