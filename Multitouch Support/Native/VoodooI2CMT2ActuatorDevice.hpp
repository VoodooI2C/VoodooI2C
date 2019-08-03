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

#ifndef EXPORT
#define EXPORT __attribute__((visibility("default")))
#endif

class EXPORT VoodooI2CMT2ActuatorDevice : public IOHIDDevice {
    OSDeclareDefaultStructors(VoodooI2CMT2ActuatorDevice);
    
public:
    IOReturn setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options);
    
    IOReturn newReportDescriptor(IOMemoryDescriptor** descriptor) const override;
    OSNumber* newVendorIDNumber() const override;
    
    
    OSNumber* newProductIDNumber() const override;
    
    
    OSNumber* newVersionNumber() const override;
    
    
    OSString* newTransportString() const override;
    
    
    OSString* newManufacturerString() const override;
    
    OSNumber* newPrimaryUsageNumber() const override;
    
    OSNumber* newPrimaryUsagePageNumber() const override;
    
    OSString* newProductString() const override;
    
    OSString* newSerialNumberString() const override;
    
    OSNumber* newLocationIDNumber() const override;
};


#endif /* VoodooI2CMT2ActuatorDevice_hpp */
