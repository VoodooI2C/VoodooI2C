//
//  VoodooI2CDeviceNub.hpp
//  VoodooI2C
//
//  Created by Alexandre on 07/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CDeviceNub_hpp
#define VoodooI2CDeviceNub_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>

class VoodooI2CDeviceNub : public IOService {
  OSDeclareDefaultStructors(VoodooI2CDeviceNub);

 public:
    // data members

    // function members
    bool attach(IOService* provider, IOService* child);
    void detach(IOService* provider);
    bool init(OSDictionary* properties);
    void free();
    bool start(IOService* provider);
    void stop(IOService* provider);

 protected:
 private:
    IOACPIPlatformDevice *acpi_device;
    bool get_device_resources();
};


#endif /* VoodooI2CDeviceNub_hpp */
