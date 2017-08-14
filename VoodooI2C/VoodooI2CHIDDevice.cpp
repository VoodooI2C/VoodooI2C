//
//  VoodooI2CDevice.cpp
//  VoodooI2C
//
//  Created by Alexandre on 02/02/2015.
//  Copyright (c) 2015 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CHIDDevice.h"
#include "VoodooI2C.h"
#include "VoodooHIDWrapper.h"

#define I2C_HID_PWR_ON  0x00
#define I2C_HID_PWR_SLEEP 0x01

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

OSDefineMetaClassAndStructors(VoodooI2CHIDDevice, VoodooI2CDevice);

bool VoodooI2CHIDDevice::attach(IOService * provider, IOService* child)
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

bool VoodooI2CHIDDevice::probe(IOService* device) {

    
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
    return 0;
}

void VoodooI2CHIDDevice::stop(IOService* device) {
    
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
    
    hid_device->workLoop->release();
    hid_device->workLoop = NULL;
    
    if (hid_device->rsize > 0){
        IOFree(hid_device->rdesc, hid_device->rsize);
        hid_device->rdesc = NULL;
        hid_device->rsize = 0;
    }
    
    IOFree(hid_device, sizeof(I2CDevice));
    
    PMstop();
    
    //hid_device->provider->close(this);
    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void VoodooI2CHIDDevice::detach( IOService * provider )
{
    assert(_controller == provider);
    _controller->release();
    _controller = 0;
    
    super::detach(provider);
}

int VoodooI2CHIDDevice::initHIDDevice(I2CDevice *hid_device) {
    PMinit();
    
    hid_device->rsize = 0;
    
    if (fetch_hid_descriptor()){
        IOLog("%s::%s::Error fetching HID Descriptor!\n", getName(), _controller->_dev->name);
        return -1;
    }
    
    reset_dev();
    
    if (fetch_report_descriptor()){
        IOLog("%s::%s::Error fetching HID Report Descriptor!\n", getName(), _controller->_dev->name);
        return -1;
    }
    
    int ret = 0;
    
    setProperty("HIDDescLength", (UInt32)hid_device->hdesc.wHIDDescLength, 32);
    setProperty("bcdVersion", (UInt32)hid_device->hdesc.bcdVersion, 32);
    setProperty("ReportDescLength", (UInt32)hid_device->hdesc.wReportDescLength, 32);
    setProperty("ReportDescRegister", (UInt32)hid_device->hdesc.wReportDescRegister, 32);
    setProperty("InputRegister", (UInt32)hid_device->hdesc.wInputRegister, 32);
    setProperty("MaxInputLength", (UInt32)hid_device->hdesc.wMaxInputLength, 32);
    setProperty("OutputRegister", (UInt32)hid_device->hdesc.wOutputRegister, 32);
    setProperty("MaxOutputLength", (UInt32)hid_device->hdesc.wMaxOutputLength, 32);
    setProperty("CommandRegister", (UInt32)hid_device->hdesc.wCommandRegister, 32);
    setProperty("DataRegister", (UInt32)hid_device->hdesc.wDataRegister, 32);
    setProperty("vendorID", (UInt32)hid_device->hdesc.wVendorID, 32);
    setProperty("productID", (UInt32)hid_device->hdesc.wProductID, 32);
    setProperty("VersionID", (UInt32)hid_device->hdesc.wVersionID, 32);
    
    
    hid_device->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!hid_device->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _controller->_dev->name);
        stop(this);
        return -1;
    }

    hid_device->workLoop->retain();
    
    hid_device->interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CHIDDevice::InterruptOccured), hid_device->provider);
    
    if (!hid_device->interruptSource) {
        goto err;
    }
    hid_device->workLoop->addEventSource(hid_device->interruptSource);
    hid_device->interruptSource->enable();
    
    hid_device->timerSource = NULL;
    
    /*hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CHIDDevice::get_input));
    if (!hid_device->timerSource){
        goto err;
    }
    
    hid_device->workLoop->addEventSource(hid_device->timerSource);
    hid_device->timerSource->setTimeoutMS(200);*/

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
    
    hid_device->deviceIsAwake = true;
    hid_device->reading = false;
    
    registerPowerDriver(this, myPowerStates, kMyNumberOfStates);
    
    return 0;
    
err:
    if (hid_device->rsize > 0){
        IOFree(hid_device->rdesc, hid_device->rsize);
        hid_device->rdesc = NULL;
        hid_device->rsize = 0;
    }
    
    PMstop();
    return ret;
}

void VoodooI2CHIDDevice::initialize_wrapper(void) {
    destroy_wrapper();

    _wrapper = new VoodooHIDWrapper;
    if (_wrapper->init()) {
        _wrapper->attach(this);
        _wrapper->start(this);
    }
    else {
        _wrapper->release();
        _wrapper = NULL;
    }
}

void VoodooI2CHIDDevice::destroy_wrapper(void) {
    if (_wrapper != NULL) {
        _wrapper->terminate(kIOServiceRequired | kIOServiceSynchronous);
        _wrapper->release();
        _wrapper = NULL;
    }
}

int VoodooI2CHIDDevice::fetch_hid_descriptor(){
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
        
int VoodooI2CHIDDevice::fetch_report_descriptor(){
    hid_device->rsize = hid_device->hdesc.wReportDescLength;
    uint8_t length = 2;
    
    union command cmd;
    cmd.c.reg = hid_device->hdesc.wReportDescRegister;
    
    writeI2C(cmd.data, length);
    
    hid_device->rdesc = (uint8_t *)IOMalloc(hid_device->rsize);
    memset(hid_device->rdesc, 0, hid_device->rsize);
    
    readI2C(hid_device->rdesc, hid_device->rsize);
    
    return 0;
};

int VoodooI2CHIDDevice::set_power(int power_state){
    uint8_t length = 4;
    
    union command cmd;
    cmd.c.reg = hid_device->hdesc.wCommandRegister;
    cmd.c.opcode = 0x08;
    cmd.c.reportTypeID = power_state;
    
    return writeI2C(cmd.data, length);
}

int VoodooI2CHIDDevice::reset_dev(){
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

IOReturn VoodooI2CHIDDevice::setPowerState(unsigned long powerState, IOService *whatDevice){
    if (powerState == 0){
        //Going to sleep
        if (hid_device->timerSource){
            hid_device->timerSource->cancelTimeout();
            hid_device->timerSource->release();
            hid_device->timerSource = NULL;
        }
        
        hid_device->deviceIsAwake = false;
        
        IOLog("%s::Going to Sleep!\n", getName());
    } else {
        if (!hid_device->deviceIsAwake){
            int ret = 0;
            
            hid_device->deviceIsAwake = true;
            IOLog("%s::Woke up from Sleep!\n", getName());
        } else {
            IOLog("%s::Device already awake! Not reinitializing.\n", getName());
        }
        
        //Waking up from Sleep
        /*if (!hid_device->timerSource){
            hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CHIDDevice::get_input));
            hid_device->workLoop->addEventSource(hid_device->timerSource);
            hid_device->timerSource->setTimeoutMS(10);
        }*/
    }
    return kIOPMAckImplied;
}

int VoodooI2CHIDDevice::i2c_hid_descriptor_address(I2CDevice *hid_device) {
    
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

int VoodooI2CHIDDevice::i2c_get_slave_address(I2CDevice* hid_device){
    OSObject* result = NULL;
    
    hid_device->provider->evaluateObject("_CRS", &result);
    
    OSData* data = OSDynamicCast(OSData, result);
    
    hid_device->addr = *(int*)data->getBytesNoCopy(16,1) & 0xFF;

    data->release();
    
    return 0;
    
}

SInt32 VoodooI2CHIDDevice::readI2C(uint8_t *values, size_t len){
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
        return ret < 0 ? ret : -1;
    return 0;
}

SInt32 VoodooI2CHIDDevice::writeI2C(uint8_t *values, size_t len){
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

void VoodooI2CHIDDevice::get_input(OSObject* owner, IOTimerEventSource* sender) {
    uint16_t maxLen = hid_device->hdesc.wMaxInputLength;
    
    unsigned char* report = (unsigned char *)IOMalloc(maxLen);
    
    int ret = readI2C(report, maxLen);

    int return_size = report[0] | report[1] << 8;
    if (return_size == 0) {
        //hid_device->timerSource->setTimeoutMS(5);
        return;
    }

    if (return_size > maxLen) {
        IOLog("%s: Incomplete report %d/%d\n", __func__, maxLen, return_size);
        return;
    }
    

    IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, return_size);
    buffer->writeBytes(0, report + 2, return_size - 2);

    IOReturn err = _wrapper->handleReport(buffer, kIOHIDReportTypeInput);
    if (err != kIOReturnSuccess)
        IOLog("Error handling report: 0x%.8x\n", err);

    buffer->release();

    IOFree(report, maxLen);
    
    //hid_device->timerSource->setTimeoutMS(5);
}

static void i2c_hid_readReport(VoodooI2CHIDDevice *device){
    device->get_input(NULL, NULL);
    
    unsigned char *report = (unsigned char *)IOMalloc(device->hid_device->hdesc.wMaxInputLength);
    device->readI2C(report, device->hid_device->hdesc.wMaxInputLength);
    IOFree(report, device->hid_device->hdesc.wMaxInputLength);
    
    device->hid_device->reading = false;
}

void VoodooI2CHIDDevice::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    if (hid_device->reading)
        return;
    
    hid_device->reading = true;
    
    thread_t newThread;
    kern_return_t kr = kernel_thread_start((thread_continue_t)i2c_hid_readReport, this, &newThread);
    if (kr != KERN_SUCCESS){
        hid_device->reading = false;
        IOLog("Thread error!\n");
    } else {
        thread_deallocate(newThread);
    }
    
}

void VoodooI2CHIDDevice::write_report_descriptor_to_buffer(IOBufferMemoryDescriptor *buffer){
    buffer->writeBytes(0, hid_device->rdesc, hid_device->rsize);
    IOLog("Report Descriptor written to buffer (%d)\n", hid_device->rsize);
}
