//
//  VoodooPrecisionTouchpadDevice.cpp
//  VoodooI2C
//
//  Created by CoolStar on 7/29/17.
//  Copyright © 2016 CoolStar. All rights reserved.
//  ported from crostrackpad-elan 3.0 for Windows
//  based off VoodooElanTouchpadDevice
//

/*
 * NOTE: THIS IS A TEMPLATE DEVICE. IT HAS STRUCTS AND REPORT ID'S THAT
 * NEED TO BE ADJUSTED FOR YOUR TOUCHPAD. IF YOU DO NOT HAVE AN ELAN0651
 * YOU NEED TO MODIFY THIS DRIVER BEFORE USING IT.
 */

#include "VoodooPrecisionTouchpadDevice.h"
#include "VoodooI2C.h"

#define I2C_HID_PWR_ON  0x00
#define I2C_HID_PWR_SLEEP 0x01

#define INPUT_MODE_MOUSE 0x00
#define INPUT_MODE_TOUCHPAD 0x03

// Begin Touchpad Specific Structs

#define CONFIDENCE_BIT 1
#define TIPSWITCH_BIT 2

#define INPUT_CONFIG_REPORT_ID 3
#define TOUCHPAD_REPORT_ID 4

struct __attribute__((__packed__)) INPUT_MODE_FEATURE {
    uint8_t ReportID;
    uint8_t Mode;
    uint8_t Reserved;
};

struct __attribute__((__packed__)) TOUCH {
    uint8_t ContactInfo; //Contact ID (4), Status (4)
    uint16_t XValue;
    uint16_t YValue;
};

struct __attribute__((__packed__)) TOUCHPAD_INPUT_REPORT {
    uint8_t ReportID;
    TOUCH MTouch;
    uint16_t ScanTime;
    uint8_t ContactCount;
    uint8_t Button;
    uint16_t Reserved;
};

// End Touchpad Specific Structs

union command {
    uint8_t data[4];
    struct __attribute__((__packed__)) cmd {
        uint16_t reg;
        uint8_t reportTypeID;
        uint8_t opcode;
    } c;
};

struct __attribute__((__packed__))  i2c_hid_cmd {
    unsigned int registerIndex;
    uint8_t opcode;
    unsigned int length;
    bool wait;
};

OSDefineMetaClassAndStructors(VoodooI2CPrecisionTouchpadDevice, VoodooI2CDevice);

void VoodooI2CPrecisionTouchpadDevice::TrackpadRawInput(struct csgesture_softc *sc, uint8_t *rawreport, int tickinc){
    struct TOUCHPAD_INPUT_REPORT *report = (struct TOUCHPAD_INPUT_REPORT *)rawreport;
    if (report->ReportID == TOUCHPAD_REPORT_ID){
        uint8_t ContactID = report->MTouch.ContactInfo >> 4;
        uint8_t ContactStatus = report->MTouch.ContactInfo & 0xF;
        
        if (ContactID >= 0 && ContactID < 15){
            if (ContactStatus & CONFIDENCE_BIT){
                if (ContactStatus & TIPSWITCH_BIT){
                    uint32_t pos_x = report->MTouch.XValue;
                    uint32_t pos_y = report->MTouch.YValue;
                    
                    pos_x /= 3;
                    pos_y /= 3;
                    
                    sc->x[ContactID] = pos_x;
                    sc->y[ContactID] = pos_y;
                    sc->p[ContactID] = 10;
                } else {
                    sc->x[ContactID] = -1;
                    sc->y[ContactID] = -1;
                    sc->p[ContactID] = -1;
                }
            }
        }
        
        sc->buttondown = report->Button & 0x1;
    }
}

bool VoodooI2CPrecisionTouchpadDevice::attach(IOService * provider, IOService* child)
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

bool VoodooI2CPrecisionTouchpadDevice::probe(IOService* device) {
    
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
    
    ret = i2c_hid_descriptor_address(hid_device);
    if (ret < 0){
        IOLog("%s::%s::Failed to get an address for a HID descriptor, aborting.\n", getName(), _controller->_dev->name);
        IOFree(hid_device, sizeof(I2CDevice));
        return false;
    }
    
    
    IOLog("%s::%s::HID Probe called for i2c 0x%02x\n", getName(), _controller->_dev->name, hid_device->addr);
    
    initHIDDevice(hid_device);
    
    //super::stop(device);
    return true;
}

void VoodooI2CPrecisionTouchpadDevice::stop(IOService* device) {
    
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
    
    if (hid_device->workLoop){
        hid_device->workLoop->release();
        hid_device->workLoop = NULL;
    }
    
    IOFree(hid_device, sizeof(I2CDevice));
    
    //hid_device->provider->close(this);
    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void VoodooI2CPrecisionTouchpadDevice::detach( IOService * provider )
{
    assert(_controller == provider);
    _controller->release();
    _controller = 0;
    
    super::detach(provider);
}

int VoodooI2CPrecisionTouchpadDevice::fetch_hid_descriptor(){
    uint8_t length = 2;
    
    union command cmd;
    cmd.c.reg = hid_device->hid_descriptor_address;
    
    writeI2C(cmd.data, length);
    
    memset(&hid_device->hdesc, 0, sizeof(i2c_hid_descr));
    
    readI2C((uint8_t *)&hid_device->hdesc, sizeof(i2c_hid_descr));
    
    IOLog("%s::%s::BCD Version: 0x%x\n", getName(), _controller->_dev->name, hid_device->hdesc.bcdVersion);
    if (hid_device->hdesc.bcdVersion == 0x0100 && hid_device->hdesc.wHIDDescLength == sizeof(i2c_hid_descr))
        return 0;
    return 1;
}

int VoodooI2CPrecisionTouchpadDevice::set_power(int power_state){
    uint8_t length = 4;
    
    union command cmd;
    cmd.c.reg = hid_device->hdesc.wCommandRegister;
    cmd.c.opcode = 0x08;
    cmd.c.reportTypeID = power_state;
    
    return writeI2C(cmd.data, length);
}

int VoodooI2CPrecisionTouchpadDevice::reset_dev(){
    set_power(I2C_HID_PWR_ON);
    
    IOSleep(1);
    
    uint8_t length = 4;
    
    union command cmd;
    cmd.c.reg = hid_device->hdesc.wCommandRegister;
    cmd.c.opcode = 0x01;
    cmd.c.reportTypeID = 0;
    
    writeI2C(cmd.data, length);
    
    IOSleep(1);
    
    uint8_t nullBytes[2];
    readI2C(nullBytes, 2);
    return 0;
}

int VoodooI2CPrecisionTouchpadDevice::write_feature(uint8_t reportID, uint8_t *buf, size_t buf_len){
    uint16_t dataReg = hid_device->hdesc.wDataRegister;
    
    uint8_t reportType = 0x03;
    
    uint8_t idx = 0;
    
    uint16_t size;
    uint16_t args_len;
    
    size = 2 +
        (reportID ? 1 : 0)	/* reportID */ +
        buf_len		/* buf */;
    args_len = (reportID >= 0x0F ? 1 : 0) /* optional third byte */ +
        2			/* dataRegister */ +
        size			/* args */;
    
    uint8_t *args = (uint8_t *)IOMalloc(args_len);
    memset(args, 0, args_len);
    
    if (reportID >= 0x0F) {
        args[idx++] = reportID;
        reportID = 0x0F;
    }
    
    args[idx++] = dataReg & 0xFF;
    args[idx++] = dataReg >> 8;
    
    args[idx++] = size & 0xFF;
    args[idx++] = size >> 8;
    
    if (reportID)
        args[idx++] = reportID;
    
    memcpy(&args[idx], buf, buf_len);
    
    uint8_t len = 4;
    union command *cmd = (union command *)IOMalloc(4 + args_len);
    memset(cmd, 0, 4+args_len);
    cmd->c.reg = hid_device->hdesc.wCommandRegister;
    cmd->c.opcode = 0x03;
    cmd->c.reportTypeID = reportID | reportType << 4;
    
    uint8_t *rawCmd = (uint8_t *)cmd;
    
    memcpy(rawCmd + len, args, args_len);
    len += args_len;
    writeI2C(rawCmd, len);
    
    IOFree(cmd, 4+args_len);
    IOFree(args, args_len);
    return 0;
}

void VoodooI2CPrecisionTouchpadDevice::enable_abs(){
    struct INPUT_MODE_FEATURE input_mode;
    input_mode.ReportID = INPUT_CONFIG_REPORT_ID;
    input_mode.Mode = INPUT_MODE_TOUCHPAD;
    input_mode.Reserved = 0x00;
    
    write_feature(input_mode.ReportID, (uint8_t *)&input_mode, sizeof(struct INPUT_MODE_FEATURE));
}

int VoodooI2CPrecisionTouchpadDevice::initHIDDevice(I2CDevice *hid_device) {
    PMinit();
    
    if (fetch_hid_descriptor()){
        IOLog("%s::%s::Error fetching HID Descriptor!\n", getName(), _controller->_dev->name);
        return -1;
    }
    
    reset_dev();
    
    enable_abs();
    
    int ret = 0;
    
    uint16_t max_x = 3209;
    uint16_t max_y = 2097;
    
    hw_res_x = 401;
    hw_res_y = 262;
    
    csgesture_softc *sc = &softc;
    
    sprintf(sc->product_id, "ELAN");
    sprintf(sc->firmware_version, "0561");
    
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
    
    sc->infoSetup = true;
    
    for (int i = 0;i < MAX_FINGERS; i++) {
        sc->x[i] = -1;
        sc->y[i] = -1;
        sc->p[i] = -1;
    }
    
    hid_device->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!hid_device->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _controller->_dev->name);
        stop(this);
        return -1;
    }
    
    hid_device->workLoop->retain();
    
    hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CPrecisionTouchpadDevice::get_input));
    if (!hid_device->timerSource){
        IOLog("%s", "Timer Err!\n");
        goto err;
    }
    
    hid_device->workLoop->addEventSource(hid_device->timerSource);
    hid_device->timerSource->setTimeoutMS(10);

    hid_device->trackpadIsAwake = true;
    
    hid_device->nullReportCount = 0;
    
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

void VoodooI2CPrecisionTouchpadDevice::initialize_wrapper(void) {
    _wrapper = new CSGesture;
    _wrapper->vendorID = 'nalE';
    _wrapper->productID = 'dptE';
    _wrapper->softc = &softc;
    _wrapper->initialize_wrapper(this);
}

void VoodooI2CPrecisionTouchpadDevice::destroy_wrapper(void) {
    _wrapper->destroy_wrapper();
}

#define EIO              5      /* I/O error */
#define ENOMEM          12      /* Out of memory */

SInt32 VoodooI2CPrecisionTouchpadDevice::readI2C(uint8_t *values, size_t len){
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = I2C_M_RD,
            .len = (uint16_t)len,
            .buf = values,
        },
    };
    int ret;
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)msgs, ARRAY_SIZE(msgs));
    if (ret != ARRAY_SIZE(msgs))
        return ret < 0 ? ret : EIO;
    return 0;
}

SInt32 VoodooI2CPrecisionTouchpadDevice::writeI2C(uint8_t *values, size_t len){
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = (uint16_t)len,
            .buf = values,
        },
    };
    int ret;
    
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)&msgs, ARRAY_SIZE(msgs));
    
    return ret;
}

IOReturn VoodooI2CPrecisionTouchpadDevice::setPowerState(unsigned long powerState, IOService *whatDevice){
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
            int ret = 0;

            hid_device->trackpadIsAwake = true;
            IOLog("%s::Woke up from Sleep!\n", getName());
        } else {
            IOLog("%s::Trackpad already awake! Not reinitializing.\n", getName());
        }

        //Waking up from Sleep
        if (!hid_device->timerSource){
            hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CPrecisionTouchpadDevice::get_input));
            hid_device->workLoop->addEventSource(hid_device->timerSource);
            hid_device->timerSource->setTimeoutMS(10);
        }
        
        if (_wrapper)
            _wrapper->wakeFromSleep();
    }
    return kIOPMAckImplied;
}

int VoodooI2CPrecisionTouchpadDevice::i2c_hid_descriptor_address(I2CDevice *hid_device) {
    
    UInt32 guid_1 = 0x3CDFF6F7;
    UInt32 guid_2 = 0x45554267;
    UInt32 guid_3 = 0x0AB305AD;
    UInt32 guid_4 = 0xDE38893D;
    
    
    OSObject *result = NULL;
    OSObject *params[3];
    char buffer[16];
    
    memcpy(buffer, &guid_1, 4);
    memcpy(buffer + 4, &guid_2, 4);
    memcpy(buffer + 8, &guid_3, 4);
    memcpy(buffer + 12, &guid_4, 4);
    
    
    params[0] = OSData::withBytes(buffer, 16);
    params[1] = OSNumber::withNumber(0x1, 8);
    params[2] = OSNumber::withNumber(0x1, 8);
    
    hid_device->provider->evaluateObject("_DSM", &result, params, 3);
    if (!result)
        hid_device->provider->evaluateObject("XDSM", &result, params, 3);
    if (!result)
        return -1;
    
    
    OSNumber* number = OSDynamicCast(OSNumber, result);
    if (number)
        hid_device->hid_descriptor_address = number->unsigned32BitValue();
    if (!number)
        return -1;
    
    if (result)
        result->release();
    
    params[0]->release();
    params[1]->release();
    params[2]->release();
    
    return 0;
}

int VoodooI2CPrecisionTouchpadDevice::i2c_get_slave_address(I2CDevice* hid_device){
    OSObject* result = NULL;
    
    hid_device->provider->evaluateObject("_CRS", &result);
    
    OSData* data = OSDynamicCast(OSData, result);
    
    hid_device->addr = *(int*)data->getBytesNoCopy(16,1) & 0xFF;
    
    data->release();
    
    return 0;
    
}

void VoodooI2CPrecisionTouchpadDevice::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    IOLog("interrupt\n");
    if (hid_device->reading)
        return;
    //i2c_hid_get_input(ihid);
}

void VoodooI2CPrecisionTouchpadDevice::readInput(int runLoop){
    //Adjust this function to suit your touchpad.
    
    uint16_t maxLen = hid_device->hdesc.wMaxInputLength;
    if (maxLen < sizeof(TOUCHPAD_INPUT_REPORT) + 2){
        maxLen = sizeof(TOUCHPAD_INPUT_REPORT) + 2;
    }
    
    uint8_t *report = (uint8_t *)IOMalloc(maxLen);
    memset(report, 0, maxLen);
    readI2C(report, maxLen);
    
    if (runLoop == 1){
        if (report[0] == 0x00 && report[1] == 0x00){
            hid_device->nullReportCount++;
            
            if (hid_device->nullReportCount >= 20){
                IOLog("%s::%s::Too many null reports. Resetting!\n", getName(), _controller->_dev->name);
                reset_dev();
                enable_abs();
                IOFree(report, maxLen);
                return;
            }
        } else {
            hid_device->nullReportCount = 0;
        }
    }
    
    if (report[2] == TOUCHPAD_REPORT_ID){
        int return_size = report[0] | report[1] << 8;
        if (return_size - 2 != sizeof(TOUCHPAD_INPUT_REPORT)){
            report[2] = 0xff; //Invalidate Report ID so it's not parsed;
        }
        
        struct TOUCHPAD_INPUT_REPORT inputReport;
        memcpy(&inputReport, report + 2, sizeof(TOUCHPAD_INPUT_REPORT));
        
        TrackpadRawInput(&softc, (uint8_t *)&inputReport, 1);
        
        if (runLoop){
            if (inputReport.ContactCount > 0){
                for (int i = 0; i < inputReport.ContactCount; i++){
                    readInput(0);
                }
            }
        }
    }
    IOFree(report, maxLen);
}

void VoodooI2CPrecisionTouchpadDevice::get_input(OSObject* owner, IOTimerEventSource* sender) {
    readInput(1);
    
    _wrapper->ProcessGesture(&softc);

    hid_device->timerSource->setTimeoutMS(10);
}
