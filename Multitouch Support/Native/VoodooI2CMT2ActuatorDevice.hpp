//
//  VoodooI2CMT2ActuatorDevice.hpp
//  VoodooI2C
//
//  Created by CoolStar on 12/19/18.
//  Copyright © 2018 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CMT2ActuatorDevice_hpp
#define VoodooI2CMT2ActuatorDevice_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#include <IOKit/hid/IOHIDDevice.h>

class VoodooI2CMT2ActuatorDevice : public IOHIDDevice {
    OSDeclareDefaultStructors(VoodooI2CMT2ActuatorDevice);
    
public:
    IOReturn setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options);
    
    virtual IOReturn newReportDescriptor(IOMemoryDescriptor** descriptor) const override;
    virtual OSNumber* newVendorIDNumber() const override;
    
    
    virtual OSNumber* newProductIDNumber() const override;
    
    
    virtual OSNumber* newVersionNumber() const override;
    
    
    virtual OSString* newTransportString() const override;
    
    
    virtual OSString* newManufacturerString() const override;
    
    virtual OSNumber* newPrimaryUsageNumber() const override;
    
    virtual OSNumber* newPrimaryUsagePageNumber() const override;
    
    virtual OSString* newProductString() const override;
    
    virtual OSString* newSerialNumberString() const override;
    
    virtual OSNumber* newLocationIDNumber() const override;
};


#endif /* VoodooI2CMT2ActuatorDevice_hpp */
