//
//  VoodooCSGestureHIDWrapper.h
//  VoodooI2C
//
//  Created by Christopher Luu on 10/7/15.
//  Copyright © 2015 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2C_VoodooCSGestureHIDWrapper_h
#define VoodooI2C_VoodooCSGestureHIDWrapper_h

#include <IOKit/hid/IOHIDDevice.h>

#ifndef EXPORT
#define EXPORT __attribute__((visibility("default")))
#endif

class VoodooI2CCSGestureEngine;

class EXPORT VoodooCSGestureHIDWrapper : public IOHIDDevice
{
    OSDeclareDefaultStructors(VoodooCSGestureHIDWrapper)
public:
    VoodooI2CCSGestureEngine *gestureEngine;
    
    bool start(IOService *provider) override;
    
    IOReturn setProperties(OSObject *properties) override;
    
    IOReturn newReportDescriptor(IOMemoryDescriptor **descriptor) const override;
    
    IOReturn setReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options=0) override;
    IOReturn getReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options) override;
    
    OSString* newManufacturerString() const override;
    OSNumber* newPrimaryUsageNumber() const override;
    OSNumber* newPrimaryUsagePageNumber() const override;
    OSNumber* newProductIDNumber() const override;
    OSString* newProductString() const override;
    OSString* newSerialNumberString() const override;
    OSString* newTransportString() const override;
    OSNumber* newVendorIDNumber() const override;
    
    OSNumber* newLocationIDNumber() const override;
};

#endif /* VoodooI2C_VoodooCSGestureHIDWrapper_h */
