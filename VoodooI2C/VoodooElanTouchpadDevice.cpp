//
//  VoodooElanTouchpadDevice.cpp
//  VoodooI2C
//
//  Created by CoolStar on 6/25/16.
//  Copyright © 2016 CoolStar. All rights reserved.
//  ported from crostrackpad-elan 3.0 for Windows
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
            /*pos_x *= 2;
            pos_x /= 7;
            pos_y *= 2;
            pos_y /= 7;*/
            pos_x *= 10;
            pos_x /= hw_res_x;
            
            pos_y *= 10;
            pos_y /= hw_res_y;
            
            
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
}

bool VoodooI2CElanTouchpadDevice::attach(IOService * provider, IOService* child)
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
    return true;
}

void VoodooI2CElanTouchpadDevice::stop(IOService* device) {
    
    IOLog("I2C HID Device is stopping\n");
    
    destroy_wrapper();
    
    if (hid_device->timerSource){
        hid_device->timerSource->cancelTimeout();
        hid_device->timerSource->release();
        hid_device->timerSource = NULL;
    }
    
    if (hid_device->interruptSource){
        hid_device->interruptSource->disable();
        hid_device->workLoop->removeEventSource(hid_device->interruptSource);
        hid_device->interruptSource->release();
        hid_device->interruptSource = NULL;
    }
    
    if (hid_device->workLoop){
        hid_device->workLoop->release();
        hid_device->workLoop = NULL;
    }
    
    IOFree(hid_device, sizeof(I2CDevice));
    
    PMstop();
    
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

static bool elan_check_ASUS_special_fw(uint8_t prodid, uint8_t ic_type)
{
    if (ic_type != 0x0E)
        return false;
    
    switch (prodid) {
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x09:
        case 0x13:
            return true;
        default:
            return false;
    }
}

int VoodooI2CElanTouchpadDevice::initHIDDevice(I2CDevice *hid_device) {
    PMinit();
    
    int ret = 0;
    
    elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_RESET);
    
    /* Wait for the device to reset */
    IOSleep(100);
    
    /* get reset acknowledgement 0000 */
    uint8_t val[256];
    readI2C(0x00, ETP_I2C_INF_LENGTH, val);
    
    readI2C16(ETP_I2C_DESC_CMD, ETP_I2C_DESC_LENGTH, val);
    
    readI2C16(ETP_I2C_REPORT_DESC_CMD, ETP_I2C_REPORT_DESC_LENGTH, val);
    
    uint8_t val2[3];
    
    elan_i2c_read_cmd(ETP_I2C_UNIQUEID_CMD, val2);
    uint8_t prodid = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_SM_VERSION_CMD, val2);
    uint8_t smvers = val2[0];
    uint8_t ictype = val2[1];
    
    if (elan_check_ASUS_special_fw(prodid, ictype)){ //some Elan trackpads on certain ASUS laptops are buggy (linux commit 2de4fcc64685def3e586856a2dc636df44532395)
        IOLog("%s::%s:: Buggy ASUS trackpad detected. Applying workaround.\n", getName(), _controller->_dev->name);
        
        elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_WAKE_UP);
        
        IOSleep(200);
        
        elan_i2c_write_cmd(ETP_I2C_SET_CMD, ETP_ENABLE_ABS);
    } else {
        elan_i2c_write_cmd(ETP_I2C_SET_CMD, ETP_ENABLE_ABS);
    
        elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_WAKE_UP);
    }
    
    elan_i2c_read_cmd(ETP_I2C_FW_VERSION_CMD, val2);
    uint8_t version = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_FW_CHECKSUM_CMD, val2);
    uint16_t csum = *((uint16_t *)val2);
    
    elan_i2c_read_cmd(ETP_I2C_IAP_VERSION_CMD, val2);
    uint8_t iapversion = val2[0];
    
    elan_i2c_read_cmd(ETP_I2C_PRESSURE_CMD, val2);
    
    elan_i2c_read_cmd(ETP_I2C_MAX_X_AXIS_CMD, val2);
    uint16_t max_x = (*((uint16_t *)val2)) & 0x0fff;
    
    elan_i2c_read_cmd(ETP_I2C_MAX_Y_AXIS_CMD, val2);
    uint16_t max_y = (*((uint16_t *)val2)) & 0x0fff;
    
    elan_i2c_read_cmd(ETP_I2C_XY_TRACENUM_CMD, val2);
    
    elan_i2c_read_cmd(ETP_I2C_RESOLUTION_CMD, val2);
    
    hw_res_x = val2[0];
    hw_res_y = val2[1];
    
    hw_res_x = (hw_res_x * 10 + 790) * 10 / 254;
    hw_res_y = (hw_res_y * 10 + 790) * 10 / 254;
    
    csgesture_softc *sc = &softc;
    
    sprintf(sc->product_id, "%d.0", prodid);
    sprintf(sc->firmware_version, "%d.0", version);
    
    sc->resx = max_x;
    sc->resy = max_y;
    
    /*sc->resx *= 2;
    sc->resx /= 7;
    
    sc->resy *= 2;
    sc->resy /= 7;*/
    
    sc->resx = max_x * 10 / hw_res_x;
    sc->resy = max_y * 10 / hw_res_y;
    
    sc->phyx = max_x;
    sc->phyy = max_y;
    
    IOLog("%s::%s::[Elan Trackpad] ProdID: %d Vers: %d Csum: %d SmVers: %d ICType: %d IAPVers: %d Max X: %d Max Y: %d\n", getName(), _controller->_dev->name, prodid, version, ictype, csum, smvers, iapversion, max_x, max_y);
    
    sc->frequency = 5;
    
    sc->infoSetup = true;
    
    hid_device->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!hid_device->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _controller->_dev->name);
        stop(this);
        return -1;
    }
    
    hid_device->workLoop->retain();
    
    hid_device->interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CElanTouchpadDevice::InterruptOccured), hid_device->provider);
    
    if (!hid_device->interruptSource) {
        IOLog("%s::%s::Interrupt Error!\n", getName(), _controller->_dev->name);
        goto err;
    }
    
    hid_device->workLoop->addEventSource(hid_device->interruptSource);
    hid_device->interruptSource->enable();
    
    hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CElanTouchpadDevice::get_input));
    if (!hid_device->timerSource){
        IOLog("%s::%s::Timer Error!\n", getName(), _controller->_dev->name);
        goto err;
    }
    
    hid_device->workLoop->addEventSource(hid_device->timerSource);
    hid_device->timerSource->setTimeoutMS(10);

    hid_device->trackpadIsAwake = true;
    hid_device->reading = false;
    
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
    _wrapper->softc = &softc;
    _wrapper->initialize_wrapper(this);
}

void VoodooI2CElanTouchpadDevice::destroy_wrapper(void) {
    if (_wrapper){
        _wrapper->destroy_wrapper();
        delete _wrapper;
        _wrapper = NULL;
    }
}

#define EIO              5      /* I/O error */
#define ENOMEM          12      /* Out of memory */

void VoodooI2CElanTouchpadDevice::elan_i2c_read_cmd(uint16_t reg, uint8_t *val) {
    readI2C16(reg, ETP_I2C_INF_LENGTH, val);
}

void VoodooI2CElanTouchpadDevice::elan_i2c_write_cmd(uint16_t reg, uint16_t cmd){
    uint16_t buf[] {
        reg,
        cmd
    };
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = sizeof(buf),
            .buf = (uint8_t *)&buf,
        }
    };
    int ret;
    
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)&msgs, ARRAY_SIZE(msgs));
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

IOReturn VoodooI2CElanTouchpadDevice::setPowerState(unsigned long powerState, IOService *whatDevice){
    if (powerState == 0){
        //Going to sleep
        if (hid_device->timerSource){
            hid_device->timerSource->cancelTimeout();
            hid_device->timerSource->release();
            hid_device->timerSource = NULL;
        }
        
        if (_wrapper)
            _wrapper->prepareToSleep();

        hid_device->trackpadIsAwake = false;
        
        IOLog("%s::Going to Sleep!\n", getName());
    } else {
        if (!hid_device->trackpadIsAwake){
            elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_RESET);
            
            /* Wait for the device to reset */
            IOSleep(100);
            
            /* get reset acknowledgement 0000 */
            uint8_t val[256];
            readI2C(0x00, ETP_I2C_INF_LENGTH, val);
            
            readI2C16(ETP_I2C_DESC_CMD, ETP_I2C_DESC_LENGTH, val);
            
            readI2C16(ETP_I2C_REPORT_DESC_CMD, ETP_I2C_REPORT_DESC_LENGTH, val);
            
            uint8_t val2[3];
            
            elan_i2c_read_cmd(ETP_I2C_UNIQUEID_CMD, val2);
            uint8_t prodid = val2[0];
            
            elan_i2c_read_cmd(ETP_I2C_SM_VERSION_CMD, val2);
            uint8_t ictype = val2[1];
            
            if (elan_check_ASUS_special_fw(prodid, ictype)){ //some Elan trackpads on certain ASUS laptops are buggy (linux commit 2de4fcc64685def3e586856a2dc636df44532395)
                IOLog("%s::%s:: Buggy ASUS trackpad detected. Applying workaround.\n", getName(), _controller->_dev->name);
                
                elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_WAKE_UP);
                
                IOSleep(200);
                
                elan_i2c_write_cmd(ETP_I2C_SET_CMD, ETP_ENABLE_ABS);
            } else {
                elan_i2c_write_cmd(ETP_I2C_SET_CMD, ETP_ENABLE_ABS);
            
                elan_i2c_write_cmd(ETP_I2C_STAND_CMD, ETP_I2C_WAKE_UP);
            }

            hid_device->trackpadIsAwake = true;
            IOLog("%s::Woke up from Sleep!\n", getName());
        } else {
            IOLog("%s::Trackpad already awake! Not reinitializing.\n", getName());
        }

        //Waking up from Sleep
        if (!hid_device->timerSource){
            hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CElanTouchpadDevice::get_input));
            hid_device->workLoop->addEventSource(hid_device->timerSource);
            hid_device->timerSource->setTimeoutMS(10);
        }
        
        if (_wrapper)
            _wrapper->wakeFromSleep();
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

static void elan_i2c_readReport(VoodooI2CElanTouchpadDevice *device){
    uint8_t report[ETP_MAX_REPORT_LEN];
    device->readI2C(0, sizeof(report), report);
    
    if (report[0] != 0xff){
        for (int i = 0;i < ETP_MAX_REPORT_LEN; i++)
            device->prevreport[i] = report[i];
    }
    
    device->TrackpadRawInput(&device->softc, device->prevreport, 1);
    device->hid_device->reading = false;
}

void VoodooI2CElanTouchpadDevice::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    if (hid_device->reading)
        return;
    
    hid_device->reading = true;
    
    thread_t newThread;
    kern_return_t kr = kernel_thread_start((thread_continue_t)elan_i2c_readReport, this, &newThread);
    if (kr != KERN_SUCCESS){
        hid_device->reading = false;
        IOLog("Thread error!\n");
    } else {
        thread_deallocate(newThread);
    }
}

void VoodooI2CElanTouchpadDevice::get_input(OSObject* owner, IOTimerEventSource* sender) {
    if (_wrapper)
        _wrapper->ProcessGesture(&softc);
    
    hid_device->timerSource->setTimeoutMS(5);
}
