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

    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    setProperty("HIDDefaultBehavior", OSString::withCString("Trackpad"));
    
    return IOHIDDevice::start(provider);
}

IOReturn VoodooHIDWrapper::setProperties(OSObject *properties) {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return kIOReturnUnsupported;
}

IOReturn VoodooHIDWrapper::newReportDescriptor(IOMemoryDescriptor **descriptor) const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, GetOwner(this)->ihid->hdesc.wReportDescLength);

    if (buffer == NULL) return kIOReturnNoResources;
    GetOwner(this)->write_report_descriptor_to_buffer(buffer);
    *descriptor = buffer;
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return kIOReturnSuccess;
}

IOReturn VoodooHIDWrapper::setReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    /*
    VoodooI2CHIDDevice* provider = OSDynamicCast(VoodooI2CHIDDevice, GetOwner(this));
    
    //if (provider->write_feature(options & 0xFF, report_data, report->getLength())) {
    //    return kIOReturnSuccess;
    //}
    return kIOReturnUnsupported;
     */
    
    UInt8* report_data = (UInt8*)IOMalloc(report->getLength());
    report->readBytes(0, report_data, report->getLength());
    
    IOLog("Multitouch library is requesting setReport:: reportID: 0x%x, report: %d, reportType: %d\n", options & 0xFF, *report_data, reportType);
    return kIOReturnSuccess;
}

IOReturn VoodooHIDWrapper::getReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) {
    /*
    VoodooI2CHIDDevice* provider = OSDynamicCast(VoodooI2CHIDDevice, GetOwner(this));

    if (provider->get_report(report, reportType, options & 0xFF)) {
        return kIOReturnSuccess;
    }
     
    //IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return kIOReturnUnsupported;
     */
    IOLog("Multitouch library is requesting getReport:: reportID: 0x%x, reportType: %d\n", options & 0xFF, reportType);
    return kIOReturnSuccess;
}

IOReturn VoodooHIDWrapper::handleReport(
                                        IOMemoryDescriptor * report,
                                        IOHIDReportType      reportType,
                                        IOOptionBits         options  ) {
    return IOHIDDevice::handleReport(report, reportType, options);
}

OSString* VoodooHIDWrapper::newManufacturerString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("Lynx Point");
}

OSNumber* VoodooHIDWrapper::newPrimaryUsageNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(0x1, 32);
}

OSNumber* VoodooHIDWrapper::newPrimaryUsagePageNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(0x2, 32);
}

OSNumber* VoodooHIDWrapper::newProductIDNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(GetOwner(this)->ihid->hdesc.wProductID, 16);
}

OSString* VoodooHIDWrapper::newProductString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("Synaptics 7500 Clearpad");
}

OSString* VoodooHIDWrapper::newSerialNumberString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("1234");
}

OSString* VoodooHIDWrapper::newTransportString() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSString::withCString("I2C");
}

OSNumber* VoodooHIDWrapper::newVendorIDNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(GetOwner(this)->ihid->hdesc.wVendorID, 16);
}

OSNumber* VoodooHIDWrapper::newLocationIDNumber() const {
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    return OSNumber::withNumber(123, 32);
}
