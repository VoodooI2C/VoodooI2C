//
//  VoodooElanTouchpadDevice.cpp
//  VoodooI2C
//
//  Created by CoolStar on 12/13/15.
//  Copyright © 2015 CoolStar. All rights reserved.
//  ported from crostrackpad-elan 3.0 beta 9.4 for Windows
//

#include "VoodooElanTouchpadDevice.h"
#include "VoodooI2C.h"

OSDefineMetaClassAndStructors(VoodooI2CElanTouchpadDevice, VoodooI2CDevice);

void VoodooI2CElanTouchpadDevice::TrackpadRawInput(struct csgesture_softc *sc, uint8_t report[ETP_MAX_REPORT_LEN], int tickinc){
    if (report[0] == 0xff) {
        return;
    }
    
    uint8_t *finger_data = &report[ETP_FINGER_DATA_OFFSET];
    int i;
    uint8_t tp_info = report[ETP_TOUCH_INFO_OFFSET];
    uint8_t hover_info = report[ETP_HOVER_INFO_OFFSET];
    bool contact_valid, hover_event;
    
    int nfingers = 0;
    
    for (int i = 0;i < MAX_FINGERS; i++) {
        sc->x[i] = -1;
        sc->y[i] = -1;
        sc->p[i] = -1;
    }
    
    hover_event = hover_info & 0x40;
    for (i = 0; i < ETP_MAX_FINGERS; i++) {
        contact_valid = tp_info & (1U << (3 + i));
        unsigned int pos_x, pos_y;
        unsigned int pressure, mk_x, mk_y;
        unsigned int area_x, area_y, major, minor;
        unsigned int scaled_pressure;
        
        if (contact_valid) {
            pos_x = ((finger_data[0] & 0xf0) << 4) |
            finger_data[1];
            pos_y = ((finger_data[0] & 0x0f) << 8) |
            finger_data[2];
            
            mk_x = (finger_data[3] & 0x0f);
            mk_y = (finger_data[3] >> 4);
            pressure = finger_data[4];
            
            //map to cypress coordinates
            //pos_y = 1500 - pos_y;
            pos_y = sc->phyy - pos_y;
            pos_x *= 2;
            pos_x /= 7;
            pos_y *= 2;
            pos_y /= 7;
            
            
            /*
             * To avoid treating large finger as palm, let's reduce the
             * width x and y per trace.
             */
            area_x = mk_x;
            area_y = mk_y;
            
            major = max(area_x, area_y);
            minor = min(area_x, area_y);
            
            scaled_pressure = pressure;
            
            if (scaled_pressure > ETP_MAX_PRESSURE)
                scaled_pressure = ETP_MAX_PRESSURE;
            sc->x[i] = pos_x;
            sc->y[i] = pos_y;
            sc->p[i] = scaled_pressure;
        }
        else {
        }
        
        if (contact_valid) {
            finger_data += ETP_FINGER_DATA_LEN;
            nfingers++;
        }
    }
    sc->buttondown = (tp_info & 0x01);
    
    _wrapper->ProcessGesture(sc);
}

bool VoodooI2CElanTouchpadDevice::attach(IOService * provider, IOService* child)
{
    if (!super::attach(provider))
        return false;
    
    assert(_controller == 0);
    _controller = (VoodooI2C*)provider;
    _controller->retain();
    
    
    child->attach(this);
    if (!probe(child))
        return false;
    
    return true;
}

bool VoodooI2CElanTouchpadDevice::probe(IOService* device) {
    
    
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

void VoodooI2CElanTouchpadDevice::stop(IOService* device) {
    
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

void VoodooI2CElanTouchpadDevice::detach( IOService * provider )
{
    assert(_controller == provider);
    _controller->release();
    _controller = 0;
    
    super::detach(provider);
}

int VoodooI2CElanTouchpadDevice::initHIDDevice(I2CDevice *hid_device) {
    PMinit();
    
    int ret;
    UInt16 hidRegister;
    
    elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_RESET);
    
    uint8_t val[256];
    readI2C(0x00, ETP_I2C_INF_LENGTH, val);
    
    readI2C16(ETP_I2C_DESC_CMD, ETP_I2C_DESC_LENGTH, val);
    
    readI2C16(ETP_I2C_REPORT_DESC_CMD, ETP_I2C_REPORT_DESC_LENGTH, val);
    
    elan_i2c_write_cmd(ETP_I2C_SET_CMD, ETP_ENABLE_ABS);
    
    elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_WAKE_UP);
    
    uint8_t val2[3];
    
    elan_i2c_read_cmd(ETP_I2C_UNIQUEID_CMD, val2);
    uint8_t prodid = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_FW_VERSION_CMD, val2);
    uint8_t version = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_FW_CHECKSUM_CMD, val2);
    uint16_t csum = *((uint16_t *)val2);
    
    elan_i2c_read_cmd(ETP_I2C_SM_VERSION_CMD, val2);
    uint8_t smvers = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_IAP_VERSION_CMD, val2);
    uint8_t iapversion = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_PRESSURE_CMD, val2);
    
    elan_i2c_read_cmd(ETP_I2C_MAX_X_AXIS_CMD, val2);
    uint16_t max_x = (*((uint16_t *)val2)) & 0x0fff;
    
    elan_i2c_read_cmd(ETP_I2C_MAX_Y_AXIS_CMD, val2);
    uint16_t max_y = (*((uint16_t *)val2)) & 0x0fff;
    
    elan_i2c_read_cmd(ETP_I2C_XY_TRACENUM_CMD, val2);
    
    uint8_t x_traces = val2[0];
    uint8_t y_traces = val2[1];
    
    csgesture_softc *sc = &softc;
    
    sprintf(sc->product_id, "%d.0", prodid);
    sprintf(sc->firmware_version, "%d.0", version);
    
    sc->resx = max_x;
    sc->resy = max_y;
    
    sc->resx *= 2;
    sc->resx /= 7;
    
    sc->resy *= 2;
    sc->resy /= 7;
    
    sc->phyx = max_x;
    sc->phyy = max_y;
    
    IOLog("%s::%s::[Elan Trackpad] ProdID: %d Vers: %d Csum: %d SmVers: %d IAPVers: %d Max X: %d Max Y: %d\n", getName(), _controller->_dev->name, prodid, version, csum, smvers, iapversion, max_x, max_y);
    
    elan_i2c_write_cmd(ETP_I2C_SET_CMD, ETP_ENABLE_CALIBRATE | ETP_ENABLE_ABS);
    
    elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_WAKE_UP);
    
    elan_i2c_write_cmd(ETP_I2C_CALIBRATE_CMD, 1);
    
    readI2C16(ETP_I2C_CALIBRATE_CMD, 1, val2);
    
    elan_i2c_write_cmd(ETP_I2C_SET_CMD, ETP_ENABLE_ABS);
    
    sc->infoSetup = true;
    
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
    
    hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CElanTouchpadDevice::get_input));
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

void VoodooI2CElanTouchpadDevice::initialize_wrapper(void) {
    _wrapper = new CSGesture;
    _wrapper->vendorID = 'nalE';
    _wrapper->productID = 'dptE';
    _wrapper->initialize_wrapper(this);
}

void VoodooI2CElanTouchpadDevice::destroy_wrapper(void) {
    _wrapper->destroy_wrapper();
}

#define EIO              5      /* I/O error */
#define ENOMEM          12      /* Out of memory */

void VoodooI2CElanTouchpadDevice::elan_i2c_read_cmd(uint16_t reg, uint8_t *val) {
    readI2C16(reg, ETP_I2C_INF_LENGTH, val);
}

void VoodooI2CElanTouchpadDevice::elan_i2c_write_cmd(uint16_t reg, uint16_t cmd){
    uint16_t buffer[] = { cmd };
    readI2C16(reg, sizeof(buffer), (uint8_t *)buffer);
}

SInt32 VoodooI2CElanTouchpadDevice::readI2C(uint8_t reg, size_t len, uint8_t *values){
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

SInt32 VoodooI2CElanTouchpadDevice::readI2C16(uint16_t reg, size_t len, uint8_t *values){
    uint16_t buf[] {
        reg
    };
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = sizeof(buf),
            .buf = (uint8_t *)&buf,
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

SInt32 VoodooI2CElanTouchpadDevice::writeI2C(uint8_t reg, size_t len, uint8_t *values){
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = 1,
            .buf = &reg,
        },
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = (uint8_t)len,
            .buf = values,
        },
    };
    int ret;
    
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)&msgs, ARRAY_SIZE(msgs));
    
    return ret;
}

SInt32 VoodooI2CElanTouchpadDevice::writeI2C16(uint16_t reg, size_t len, uint8_t *values){
    uint16_t buf[] {
        reg
    };
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = sizeof(buf),
            .buf = (uint8_t *)&buf,
        },
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = (uint8_t)len,
            .buf = values,
        },
    };
    int ret;
    
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)&msgs, ARRAY_SIZE(msgs));
    
    return ret;
}

IOReturn VoodooI2CElanTouchpadDevice::setPowerState(unsigned long powerState, IOService *whatDevice){
    if (powerState == 0){
        //Going to sleep
        if (hid_device->timerSource){
            hid_device->timerSource->cancelTimeout();
            hid_device->timerSource->release();
            hid_device->timerSource = NULL;
        }
        IOLog("%s::Going to Sleep!\n", getName());
    } else {
        //Waking up from Sleep
        if (!hid_device->timerSource){
            hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CElanTouchpadDevice::get_input));
            hid_device->workLoop->addEventSource(hid_device->timerSource);
            hid_device->timerSource->setTimeoutMS(10);
        }
        
        IOLog("%s::Woke up from Sleep!\n", getName());
    }
    return kIOPMAckImplied;
}

int VoodooI2CElanTouchpadDevice::i2c_get_slave_address(I2CDevice* hid_device){
    OSObject* result = NULL;
    
    hid_device->provider->evaluateObject("_CRS", &result);
    
    OSData* data = OSDynamicCast(OSData, result);
    
    hid_device->addr = *(int*)data->getBytesNoCopy(16,1) & 0xFF;
    
    data->release();
    
    return 0;
    
}

void VoodooI2CElanTouchpadDevice::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    IOLog("interrupt\n");
    if (hid_device->reading)
        return;
    //i2c_hid_get_input(ihid);
}

void VoodooI2CElanTouchpadDevice::get_input(OSObject* owner, IOTimerEventSource* sender) {
    uint8_t report[ETP_MAX_REPORT_LEN];
    readI2C(0, sizeof(report), report);
    
    if (report[0] != 0xff){
        for (int i = 0;i < ETP_MAX_REPORT_LEN; i++)
            prevreport[i] = report[i];
    }
    
    TrackpadRawInput(&softc, prevreport, 1);
    hid_device->timerSource->setTimeoutMS(10);
}