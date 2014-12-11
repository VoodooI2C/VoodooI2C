#include "VoodooI2CRMI4Device.h"

#define super IOService

OSDefineMetaClassAndStructors(VoodooI2CRMI4Device, IOService);

bool VoodooI2CRMI4Device::init() {
    if(!super::init())
        return false;
    
    return true;
}

bool VoodooI2CRMI4Device::start(IOService* provider) {
    IOLog("%s::Found RMI4 device %s\n", getName(), getMatchedName(provider));
    if(!super::start(provider))
        return false;
    
    fACPIDevice = OSDynamicCast(IOACPIPlatformDevice, provider);
    
    if (!fACPIDevice)
        return false;
    
    _dev = (I2CDevice *)IOMalloc(sizeof(I2CDevice));
    _dev->provider = fACPIDevice;
    _dev->name = getMatchedName(fACPIDevice);
    
    _dev->provider->retain();
    
    _dev->bus_provider = VoodooI2C::I2CBus1;
    
    
    
    if(!_dev->provider->open(this)) {
        IOLog("%s::%s::Failed to open ACPI device\n", getName(), _dev->name);
        return false;
    }
    
    
    _dev->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!_dev->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _dev->name);
        VoodooI2CRMI4Device::stop();
        return false;
    }
    
    _dev->workLoop->retain();
    
    
    _dev->interruptSource =
    IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CRMI4Device::interruptOccured));
    
    if (_dev->workLoop->addEventSource(_dev->interruptSource) != kIOReturnSuccess) {
        IOLog("%s::%s::Could not add interrupt source to workloop\n", getName(), _dev->name);
        stop();
        return false;
    }
    
    _dev->interruptSource->enable();

    
    registerService();
    
    return true;
}


void VoodooI2CRMI4Device::stop() {
    
    _dev->workLoop->removeEventSource(_dev->interruptSource);
    _dev->interruptSource->disable();
    _dev->interruptSource = NULL;
    
    _dev->workLoop->release();
    _dev->workLoop = NULL;
    
    _dev->provider->close(this);
    OSSafeReleaseNULL(_dev->provider);
    
    IOFree(_dev, sizeof(I2CDevice));
     

    
    super::stop(this);
}

char* VoodooI2CRMI4Device::getMatchedName(IOService* provider) {
        OSData *data;
        data = OSDynamicCast(OSData, provider->getProperty("name"));
        return (char*)data->getBytesNoCopy();
}

void VoodooI2CRMI4Device::interruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount) {
    IOLog("%s::%s::Interrupt Occured\n", getName(), _dev->name);
}

int VoodooI2CRMI4Device::rmi_set_page(I2CDevice *phys, UInt page) {
    rmi_i2c_data *data = phys->data;
    int rc;
    
    rc = VoodooI2C::i2c_smbus_write_byte_data(phys->bus_provider, phys->addr, RMI_PAGE_SELECT_REGISTER, page);
    if (rc < 0) {
        IOLog("%s::%s::Set page failed: %d\n", getName(), _dev->name, rc);
        return rc;
    }
    data->page = page;
    return 0;
}

int VoodooI2CRMI4Device::rmi_i2c_write_block(I2CDevice *phys, UInt16 addr, UInt8 *buf, int len) {
    int rc;
    rmi_i2c_data *data = phys->data;
    
    IOLockLock(data->page_mutex);
    
    if (RMI_I2C_PAGE(addr) != data->page) {
        rc = rmi_set_page(_dev, RMI_I2C_PAGE(addr));
        if (rc < 0)
            goto exit;
    }
    
    rc = VoodooI2C::i2c_smbus_write_i2c_block_data(phys->bus_provider, addr & 0xff, sizeof(buf), buf);
    
exit:
    IOLockUnlock(data->page_mutex);
    return rc;
}

int VoodooI2CRMI4Device::rmi_i2c_write(I2CDevice *phys, UInt16 addr, UInt8 data) {
    int rc = rmi_i2c_write_block(phys, addr, &data, 1);
    return (rc < 0) ? rc : 0;
}
