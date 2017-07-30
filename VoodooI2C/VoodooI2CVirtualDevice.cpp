//
//  VoodooI2CVirtualDevice.cpp
//  VoodooI2C
//
//  Created by Kishor Prins on 2017-07-30.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CVirtualDevice.h"

#define USB_VENDOR_ID_APPLE		0x05ac
#define USB_DEVICE_ID_APPLE_WELLSPRING8_ISO	0x0291

bool VoodooI2CVirtualDevice::start(IOService *provider) {
    return IOHIDDevice::start(provider);
}

void VoodooI2CVirtualDevice::stop(IOService *provider) {
    IOHIDDevice::stop(provider);
}

OSNumber* VoodooI2CVirtualDevice::newProductIDNumber() const {
    return OSNumber::withNumber(USB_DEVICE_ID_APPLE_WELLSPRING8_ISO, 32);
}
OSNumber* VoodooI2CVirtualDevice::newVendorIDNumber() const {
    return OSNumber::withNumber(USB_VENDOR_ID_APPLE, 32);
}

