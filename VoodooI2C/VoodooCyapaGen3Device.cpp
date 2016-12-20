//
//  VoodooCyapaGen3Device.cpp
//  VoodooI2C
//
//  Created by CoolStar on 12/13/15.
//  Copyright © 2015 CoolStar. All rights reserved.
//  ported from crostrackpad 3.0 for Windows
//

#include "VoodooCyapaGen3Device.h"
#include "VoodooI2C.h"

OSDefineMetaClassAndStructors(VoodooI2CCyapaGen3Device, VoodooI2CDevice);

void VoodooI2CCyapaGen3Device::TrackpadRawInput(struct csgesture_softc *sc, cyapa_regs *regs, int tickinc){
    int nfingers;
    
    if ((regs->stat & CYAPA_STAT_RUNNING) == 0) {
        regs->fngr = 0;
    }
    
    nfingers = CYAPA_FNGR_NUMFINGERS(regs->fngr);
    
    for (int i = 0;i < 15;i++) {
        sc->x[i] = -1;
        sc->y[i] = -1;
        sc->p[i] = -1;
    }
    for (int i = 0;i < nfingers;i++) {
        int a = regs->touch[i].id;
        int x = CYAPA_TOUCH_X(regs, i);
        int y = CYAPA_TOUCH_Y(regs, i);
        int p = CYAPA_TOUCH_P(regs, i);
        sc->x[a] = x;
        sc->y[a] = y;
        sc->p[a] = p;
    }
    
    sc->buttondown = (regs->fngr & CYAPA_FNGR_LEFT);
    
    _wrapper->ProcessGesture(sc);
}

bool VoodooI2CCyapaGen3Device::attach(IOService * provider, IOService* child)
{
    if (!super::attach(provider))
        return false;

    _controller = (VoodooI2C*)provider;
    _controller->retain();
    
    
    child->attach(this);
    if (!probe(child))
        return false;
    
    return true;
}

bool VoodooI2CCyapaGen3Device::probe(IOService* device) {
    
    
    hid_device = (I2CDevice *)IOMalloc(sizeof(I2CDevice));
    
    //hid_device->_dev = _controller->_dev;
    
    if (!super::start(device))
        return false;
    
    
    hid_device->provider = OSDynamicCast(IOACPIPlatformDevice, device);
    hid_device->provider->retain();
    
    int ret = i2c_get_slave_address(hid_device);
    if (ret < 0){
        IOLog("%s::%s::Failed to get a slave address for an I2C device, aborting.\n", getName(), _controller->_dev->name);
        IOFree(hid_device, sizeof(I2CDevice));
        return false;
    }
    
    
    IOLog("%s::%s::HID Probe called for i2c 0x%02x\n", getName(), _controller->_dev->name, hid_device->addr);
    
    initHIDDevice(hid_device);
    
    //super::stop(device);
    return 0;
}

void VoodooI2CCyapaGen3Device::stop(IOService* device) {
    
    IOLog("I2C HID Device is stopping\n");
    
    destroy_wrapper();
    
    if (hid_device->timerSource){
        hid_device->timerSource->cancelTimeout();
        hid_device->timerSource->release();
        hid_device->timerSource = NULL;
    }
    
    //hid_device->workLoop->removeEventSource(hid_device->interruptSource);
    //hid_device->interruptSource->disable();
    hid_device->interruptSource = NULL;
    
    hid_device->workLoop->release();
    hid_device->workLoop = NULL;
    
    IOFree(hid_device, sizeof(I2CDevice));
    
    //hid_device->provider->close(this);
    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void VoodooI2CCyapaGen3Device::detach( IOService * provider )
{
    assert(_controller == provider);
    _controller->release();
    _controller = 0;
    
    super::detach(provider);
}

void VoodooI2CCyapaGen3Device::cyapa_set_power_mode(uint8_t power_mode)
{
    uint8_t ret;
    uint8_t power;
    
    readI2C(CMD_POWER_MODE, 1, &ret);
    if (ret < 0)
        return;
    
    power = (ret & ~0xFC);
    power |= power_mode & 0xFc;
    
    uint8_t buf[2] = {CMD_POWER_MODE, power};
    
    writeI2C(sizeof(buf), buf);
}

int VoodooI2CCyapaGen3Device::initHIDDevice(I2CDevice *hid_device) {
    PMinit();
    
    int ret = 0;
    
    uint8_t bl_exit[] = {
        0x00, 0x00, 0xff, 0xa5, 0x00, 0x01,
        0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    
    uint8_t bl_deactivate[] = {
        0x00, 0xff, 0x3b, 0x00, 0x01,
        0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
    
    cyapa_boot_regs boot;
    
    readI2C(0x00, sizeof(boot), (uint8_t *)&boot);
    
    if ((boot.stat & CYAPA_STAT_RUNNING) == 0){
        if (boot.error & CYAPA_ERROR_BOOTLOADER)
            ret = writeI2C(sizeof(bl_deactivate), bl_deactivate);
        else
            ret = writeI2C(sizeof(bl_exit), bl_exit);
    }
    
    cyapa_cap cap;
    readI2C(CMD_QUERY_CAPABILITIES, sizeof(cap), (uint8_t *)&cap);
    if (strncmp((const char *)cap.prod_ida, "CYTRA", 5) != 0) {
        IOLog("%s::%s::[cyapainit] Product ID \"%5.5s\" mismatch\n", getName(), _controller->_dev->name,
                   cap.prod_ida);
    }
    
    csgesture_softc *sc = &softc;
    sc->resx = ((cap.max_abs_xy_high << 4) & 0x0F00) |
    cap.max_abs_x_low;
    sc->resy = ((cap.max_abs_xy_high << 8) & 0x0F00) |
    cap.max_abs_y_low;
    sc->phyx = ((cap.phy_siz_xy_high << 4) & 0x0F00) |
    cap.phy_siz_x_low;
    sc->phyy = ((cap.phy_siz_xy_high << 8) & 0x0F00) |
    cap.phy_siz_y_low;
    IOLog("%s::%s::[cyapainit] %5.5s-%6.6s-%2.2s buttons=%c%c%c res=%dx%d\n",
               getName(), _controller->_dev->name,
               cap.prod_ida, cap.prod_idb, cap.prod_idc,
               ((cap.buttons & CYAPA_FNGR_LEFT) ? 'L' : '-'),
               ((cap.buttons & CYAPA_FNGR_MIDDLE) ? 'M' : '-'),
               ((cap.buttons & CYAPA_FNGR_RIGHT) ? 'R' : '-'),
               sc->resx,
               sc->resy);
    
    for (int i = 0; i < 5; i++) {
        sc->product_id[i] = cap.prod_ida[i];
    }
    sc->product_id[5] = '-';
    for (int i = 0; i < 6; i++) {
        sc->product_id[i + 6] = cap.prod_idb[i];
    }
    sc->product_id[12] = '-';
    for (int i = 0; i < 2; i++) {
        sc->product_id[i + 13] = cap.prod_idc[i];
    }
    sc->product_id[15] = '\0';
    
    sprintf(sc->firmware_version, "%d.%d", cap.fw_maj_ver, cap.fw_min_ver);
    sc->infoSetup = true;
    
    cyapa_set_power_mode(CMD_POWER_MODE_FULL);

    
    hid_device->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!hid_device->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _controller->_dev->name);
        stop(this);
        return -1;
    }
    
    hid_device->workLoop->retain();
    
    /*
     hid_device->interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CHIDDevice::InterruptOccured), hid_device->provider);
     
     if (hid_device->workLoop->addEventSource(hid_device->interruptSource) != kIOReturnSuccess) {
     IOLog("%s::%s::Could not add interrupt source to workloop\n", getName(), _controller->_dev->name);
     stop(this);
     return -1;
     }
     
     hid_device->interruptSource->enable();
     */
    
    hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CCyapaGen3Device::get_input));
    if (!hid_device->timerSource){
        IOLog("%s", "Timer Err!\n");
        goto err;
    }
    
    hid_device->workLoop->addEventSource(hid_device->timerSource);
    hid_device->timerSource->setTimeoutMS(10);
    /*
     
     hid_device->commandGate = IOCommandGate::commandGate(this);
     
     if (!hid_device->commandGate || (_dev->workLoop->addEventSource(hid_device->commandGate) != kIOReturnSuccess)) {
     IOLog("%s::%s::Failed to open HID command gate\n", getName(), _dev->name);
     return -1;
     }
     */
    
    initialize_wrapper();
    registerService();
    
#define kMyNumberOfStates 2
    
    static IOPMPowerState myPowerStates[kMyNumberOfStates];
    // Zero-fill the structures.
    bzero (myPowerStates, sizeof(myPowerStates));
    // Fill in the information about your device's off state:
    myPowerStates[0].version = 1;
    myPowerStates[0].capabilityFlags = kIOPMPowerOff;
    myPowerStates[0].outputPowerCharacter = kIOPMPowerOff;
    myPowerStates[0].inputPowerRequirement = kIOPMPowerOff;
    // Fill in the information about your device's on state:
    myPowerStates[1].version = 1;
    myPowerStates[1].capabilityFlags = kIOPMPowerOn;
    myPowerStates[1].outputPowerCharacter = kIOPMPowerOn;
    myPowerStates[1].inputPowerRequirement = kIOPMPowerOn;
    
    
    
    this->_controller->joinPMtree(this);
    
    registerPowerDriver(this, myPowerStates, kMyNumberOfStates);
    
    return 0;
    
err:
    return ret;
}

void VoodooI2CCyapaGen3Device::initialize_wrapper(void) {
    _wrapper = new CSGesture;
    _wrapper->vendorID = 'pyaC';
    _wrapper->productID = 'rtyC';
    _wrapper->softc = &softc;
    _wrapper->initialize_wrapper(this);
}

void VoodooI2CCyapaGen3Device::destroy_wrapper(void) {
    _wrapper->destroy_wrapper();
}

#define EIO              5      /* I/O error */
#define ENOMEM          12      /* Out of memory */

SInt32 VoodooI2CCyapaGen3Device::readI2C(uint8_t reg, size_t len, uint8_t *values){
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = 1,
            .buf = &reg,
        },
        {
            .addr = hid_device->addr,
            .flags = I2C_M_RD,
            .len = (uint8_t)len,
            .buf = values,
        },
    };
    int ret;
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)msgs, ARRAY_SIZE(msgs));
    if (ret != ARRAY_SIZE(msgs))
        return ret < 0 ? ret : EIO;
    return 0;
}

SInt32 VoodooI2CCyapaGen3Device::writeI2C(size_t len, uint8_t *values){
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = (uint8_t)len,
            .buf = values,
        }
    };
    int ret;
    
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)&msgs, ARRAY_SIZE(msgs));
    
    return ret;
}

IOReturn VoodooI2CCyapaGen3Device::setPowerState(unsigned long powerState, IOService *whatDevice){
    if (powerState == 0){
        //Going to sleep
        if (hid_device->timerSource){
            hid_device->timerSource->cancelTimeout();
            hid_device->timerSource->release();
            hid_device->timerSource = NULL;
        }
        
        if (_wrapper)
            _wrapper->prepareToSleep();
        
        IOLog("%s::Going to Sleep!\n", getName());
    } else {
        //Waking up from Sleep
        if (!hid_device->timerSource){
            hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CCyapaGen3Device::get_input));
            hid_device->workLoop->addEventSource(hid_device->timerSource);
            hid_device->timerSource->setTimeoutMS(10);
        }
        
        if (_wrapper)
            _wrapper->wakeFromSleep();
        
        IOLog("%s::Woke up from Sleep!\n", getName());
    }
    return kIOPMAckImplied;
}

int VoodooI2CCyapaGen3Device::i2c_get_slave_address(I2CDevice* hid_device){
    OSObject* result = NULL;
    
    hid_device->provider->evaluateObject("_CRS", &result);
    
    OSData* data = OSDynamicCast(OSData, result);
    
    hid_device->addr = *(int*)data->getBytesNoCopy(16,1) & 0xFF;
    
    data->release();
    
    return 0;
    
}

void VoodooI2CCyapaGen3Device::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    IOLog("interrupt\n");
    if (hid_device->reading)
        return;
    //i2c_hid_get_input(ihid);
}

void VoodooI2CCyapaGen3Device::get_input(OSObject* owner, IOTimerEventSource* sender) {
    cyapa_regs regs;
    readI2C(0, sizeof(regs), (uint8_t *)&regs);
    
    TrackpadRawInput(&softc, &regs, 1);
    hid_device->timerSource->setTimeoutMS(10);
}
