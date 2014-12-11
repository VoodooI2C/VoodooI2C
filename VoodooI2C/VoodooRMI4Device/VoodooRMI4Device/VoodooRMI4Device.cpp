#include "VoodooRMI4Device.h"

#define super IOService
OSDefineMetaClassAndStructors(VoodooRMI4Device, IOService);

bool VoodooRMI4Device::start(IOService * provider) {
    IOLog("%s::Found RMI4 Device %s\n", getName(), getMatchedName(provider));
    
    provider->getParent.
    
    fACPIDevice = OSDynamicCast(IOACPIPlatformDevice, provider);

    return false;
}

char* VoodooRMI4Device::getMatchedName(IOService* provider) {
    OSData *data;
    data = OSDynamicCast(OSData, provider->getProperty("name"));
    return (char*)data->getBytesNoCopy();
}

