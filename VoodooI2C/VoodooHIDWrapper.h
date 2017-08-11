//
//  VoodooHIDWrapper.h
//  VoodooI2C
//
//  Created by Christopher Luu on 10/7/15.
//  Copyright © 2015 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2C_VoodooHIDWrapper_h
#define VoodooI2C_VoodooHIDWrapper_h

#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDInterface.h>

class VoodooI2CHIDDevice;

class VoodooHIDWrapper : public IOHIDDevice
{
    OSDeclareDefaultStructors(VoodooHIDWrapper)

public:
    virtual bool start(IOService *provider);
    
    virtual IOReturn setProperties(OSObject *properties);
    
    virtual IOReturn newReportDescriptor(IOMemoryDescriptor **descriptor) const;
    
    virtual IOReturn setReport(IOMemoryDescriptor *report,IOHIDReportType reportType,IOOptionBits options=0);
    virtual IOReturn getReport(IOMemoryDescriptor *report,IOHIDReportType reportType,IOOptionBits options);
    virtual IOReturn handleReport(
                                  IOMemoryDescriptor * report,
                                  IOHIDReportType      reportType = kIOHIDReportTypeInput,
                                  IOOptionBits         options    = 0 );
    
    virtual OSString* newManufacturerString() const;
    virtual OSNumber* newPrimaryUsageNumber() const;
    virtual OSNumber* newPrimaryUsagePageNumber() const;
    virtual OSNumber* newProductIDNumber() const;
    virtual OSString* newProductString() const;
    virtual OSString* newSerialNumberString() const;
    virtual OSString* newTransportString() const;
    virtual OSNumber* newVendorIDNumber() const;
    
    virtual OSNumber* newLocationIDNumber() const;
    
    IOHIDInterface* interface;
};

#endif /* VoodooI2C_VoodooHIDWrapper_h */
