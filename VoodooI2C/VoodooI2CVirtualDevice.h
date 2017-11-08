//
//  VoodooI2CVirtualDevice.h
//  VoodooI2C
//
//  Created by Kishor Prins on 2017-07-30.
//  Copyright © 2017 Kishor Prins. All rights reserved.
//

#ifndef VOODOOI2C_VIRTUAL_DEVICE_H
#define VOODOOI2C_VIRTUAL_DEVICE_H

#include "IOKit/hid/IOHIDDevice.h"

class VoodooI2CVirtualDevice : public IOHIDDevice {
    OSDeclareDefaultStructors(VoodooI2CVirtualDevice)
public:
    void free(void) override;

    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    
    OSNumber* newProductIDNumber() const override;
    OSNumber* newVendorIDNumber() const override;
};


#endif /* VoodooI2CVirtualDevice_h */
