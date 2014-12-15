//
//  RMI4Device.cpp
//  VoodooI2C
//
//  Created by Alexandre on 12/12/2014.
//  Copyright (c) 2014 Alexandre Daoud. All rights reserved.
//

#include "VoodooRMI4Device.h"
//#include "VoodooI2C.h"

#define super IOService
OSDefineMetaClassAndStructors(VoodooRMI4Device, IOService);

bool VoodooRMI4Device::start(IOService* provider) {
    if(!super::start(provider))
        return false;
    /*
    IOACPIPlatformDevice *fACPIDevice = OSDynamicCast(IOACPIPlatformDevice, provider);
    
    if (!fACPIDevice)
        return false;
    
    _rmi4dev = (RMI4Device *)IOMalloc(sizeof(RMI4Device));
    
    _rmi4dev->provider = fACPIDevice;
    //_rmi4dev->name = (char*)_rmi4dev->provider->getProperty("name");
    
    _rmi4dev->provider->retain();
    
    if(!_rmi4dev->provider->open(this)) {
        IOLog("%s::%s::Failed to open ACPI device\n", getName(), _rmi4dev->name);
        return false;
    }
    */
    registerService();

    return true;
}

void VoodooRMI4Device::stop() {
    
    
    //_rmi4dev->provider->close(this);
    
   // _rmi4dev->provider->release();
    
    //OSSafeReleaseNULL(_rmi4dev->provider);
    
    //IOFree(_rmi4dev, sizeof(RMI4Device));
    
    super::stop(this);
}