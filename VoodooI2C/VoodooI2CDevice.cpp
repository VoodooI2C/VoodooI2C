//
//  VoodooI2CDevice.cpp
//  VoodooI2C
//
//  Created by Alexandre on 02/02/2015.
//  Copyright (c) 2015 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CHIDDevice.h"
#include "VoodooI2C.h"

OSDefineMetaClassAndStructors(VoodooI2CHIDDevice, IOService);

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
    
    
    hid_device->addr = 0x002c;
    //hid_device->_dev = _controller->_dev;

    IOLog("%s::%s::HID Probe called for i2c 0x%02x\n", getName(), _controller->_dev->name, hid_device->addr);
    
    if (!super::start(device))
        return false;
    
    
    hid_device->provider = OSDynamicCast(IOACPIPlatformDevice, device);
    hid_device->provider->retain();

    
    initHIDDevice(hid_device);
    
    //super::stop(device);
    return 0;
}

void VoodooI2CHIDDevice::stop(IOService* device) {
    
    IOLog("I2C HID Device is stopping\n");
    
    hid_device->workLoop->removeEventSource(hid_device->interruptSource);
    hid_device->interruptSource->disable();
    hid_device->interruptSource = NULL;
    
    hid_device->workLoop->release();
    hid_device->workLoop = NULL;
    
    i2c_hid_free_buffers(ihid, HID_MIN_BUFFER_SIZE);
    IOFree(ihid, sizeof(i2c_hid));
    
    IOFree(hid_device, sizeof(I2CDevice));
    
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
    int ret;
    UInt16 hidRegister;
    
    ihid = (i2c_hid*)IOMalloc(sizeof(i2c_hid));
    
    ihid->client = hid_device;
    
    ret = i2c_hid_acpi_pdata(ihid);
    
    ihid->client = hid_device;
    
    
    hidRegister = ihid->pdata.hid_descriptor_address;
    
    ihid->wHIDDescRegister = (__le16)hidRegister;
    
    IOLog("register: 0x%x\n", ihid->wHIDDescRegister);
    
    ret = i2c_hid_alloc_buffers(ihid, HID_MIN_BUFFER_SIZE);
    if (ret < 0)
        goto err;
    /*
    ret = i2c_hid_set_power(ihid, I2C_HID_PWR_ON);
    if(ret<0)
        goto err;
    */
    
    ret = i2c_hid_fetch_hid_descriptor(ihid);
    if (ret < 0)
        goto err;
    
    setProperty("HIDDescLength", (UInt32)ihid->hdesc.wHIDDescLength, 32);
    setProperty("bcdVersion", (UInt32)ihid->hdesc.bcdVersion, 32);
    setProperty("ReportDescLength", (UInt32)ihid->hdesc.wReportDescLength, 32);
    setProperty("ReportDescRegister", (UInt32)ihid->hdesc.wReportDescRegister, 32);
    setProperty("InputRegister", (UInt32)ihid->hdesc.wInputRegister, 32);
    setProperty("MaxInputRegister", (UInt32)ihid->hdesc.wMaxInputRegister, 32);
    setProperty("OutputRegister", (UInt32)ihid->hdesc.wOutputRegister, 32);
    setProperty("MaxOutputLength", (UInt32)ihid->hdesc.wMaxOutputLength, 32);
    setProperty("CommandRegister", (UInt32)ihid->hdesc.wCommandRegister, 32);
    setProperty("DataRegister", (UInt32)ihid->hdesc.wDataRegister, 32);
    setProperty("vendorID", (UInt32)ihid->hdesc.wVendorID, 32);
    setProperty("productID", (UInt32)ihid->hdesc.wProductID, 32);
    setProperty("VersionID", (UInt32)ihid->hdesc.wVersionID, 32);
    
    
    hid_device->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!hid_device->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _controller->_dev->name);
        stop(this);
        return -1;
    }

    hid_device->workLoop->retain();
    
    
     hid_device->interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CHIDDevice::InterruptOccured), hid_device->provider);
     
     if (hid_device->workLoop->addEventSource(hid_device->interruptSource) != kIOReturnSuccess) {
         IOLog("%s::%s::Could not add interrupt source to workloop\n", getName(), _controller->_dev->name);
         stop(this);
         return -1;
     }
     
     hid_device->interruptSource->enable();
    
    
    i2c_hid_get_report_descriptor(ihid);
     
     
     /*
     
     hid_device->commandGate = IOCommandGate::commandGate(this);
     
     if (!hid_device->commandGate || (_dev->workLoop->addEventSource(hid_device->commandGate) != kIOReturnSuccess)) {
     IOLog("%s::%s::Failed to open HID command gate\n", getName(), _dev->name);
     return -1;
     }
     */
    
    registerService();
    
    return 0;
    
err:
    i2c_hid_free_buffers(ihid, HID_MIN_BUFFER_SIZE);
    IOFree(ihid, sizeof(i2c_hid));
    return ret;
}

int VoodooI2CHIDDevice::i2c_hid_acpi_pdata(i2c_hid *ihid) {
    //static UInt8 i2c_hid_guid[] = {
    //     0xF7, 0xF6, 0xDF, 0x3C, 0x67, 0x42, 0x55, 0x45,
    //    0xAD, 0x05, 0xB3, 0x0A, 0x3D, 0x89, 0x38, 0xDE,
    // };
    
    ihid->pdata.hid_descriptor_address = 0x20;//ihid->client->provider->evaluateInteger("_DSM", &result);
    
    return 0;
}

int VoodooI2CHIDDevice::i2c_hid_alloc_buffers(i2c_hid *ihid, UInt report_size) {
    int args_len = sizeof(UInt8) + sizeof(UInt16) + sizeof(UInt16) + report_size;
    
    ihid->inbuf = (char *)IOMalloc(report_size);
    ihid->argsbuf = (char *)IOMalloc(report_size);
    ihid->cmdbuf = (char *)IOMalloc(sizeof(union command) + args_len);
    
    if(!ihid->inbuf || !ihid->argsbuf || !ihid->cmdbuf) {
        i2c_hid_free_buffers(ihid, report_size);
        return -1;
    }
    
    ihid->bufsize = report_size;
    
    return 0;
}

void VoodooI2CHIDDevice::i2c_hid_free_buffers(i2c_hid *ihid, UInt report_size) {
    IOFree(ihid->inbuf, report_size);
    IOFree(ihid->argsbuf, report_size);
    IOFree(ihid->cmdbuf, sizeof(UInt8) + sizeof(UInt16) + sizeof(UInt16) + report_size);
    ihid->inbuf = NULL;
    ihid->cmdbuf = NULL;
    ihid->argsbuf = NULL;
    ihid->bufsize = 0;
}

int VoodooI2CHIDDevice::i2c_hid_fetch_hid_descriptor(i2c_hid *ihid) {
    struct i2c_hid_desc *hdesc = &ihid->hdesc;
    UInt dsize;
    int ret;
    
    ret = i2c_hid_command(ihid, &hid_descr_cmd, ihid->hdesc_buffer, sizeof(struct i2c_hid_desc));
    
    if (ret)
    {
        IOLog("%s::%s::hid_descr_cmd failed\n", getName(), hid_device->name);
        return -1;
    }
    
    if((UInt16)(hdesc->bcdVersion) != 0x0100) {
        IOLog("%s::%s::Unexpected HID descriptor bcdVersion %x\n", getName(), hid_device->name, (UInt16)(hdesc->bcdVersion));
        return -1;
    }
    
    //dsize = (UInt16)(hdesc->wHIDDescLength);
    
    //if (dsize != sizeof(struct i2c_hid_desc)) {
    //    IOLog("%s::%s::weird size of HID descriptor\n", getName(), _dev->name);
    //    return -1;
    //}
    
    return 0;
}

int VoodooI2CHIDDevice::i2c_hid_command(i2c_hid *ihid, struct i2c_hid_cmd *command, unsigned char *buf_recv, int data_len) {
    return __i2c_hid_command(ihid, command, 0, 0, NULL, 0, buf_recv, data_len);
}

int VoodooI2CHIDDevice::__i2c_hid_command(i2c_hid *ihid, struct i2c_hid_cmd *command, UInt8 reportID, UInt8 reportType, UInt8 *args, int args_len, unsigned char *buf_recv, int data_len) {
    union command *cmd = (union command *)ihid->cmdbuf;
    int ret;
    struct i2c_msg msg[2];
    int msg_num = 1;
    
    int length = command->length;
    bool wait = command->wait;
    UInt registerIndex = command->registerIndex;
    
    if (command == &hid_descr_cmd) {
        cmd->c.reg = ihid->wHIDDescRegister;
    } else {
        cmd->data[0] = ihid->hdesc_buffer[registerIndex];
        cmd->data[1] = ihid->hdesc_buffer[registerIndex + 1];
    }
    
    if (length > 2) {
        cmd->c.opcode = command->opcode;
        cmd->c.reportTypeID = reportID | reportType << 4;
    }
    
    memcpy(cmd->data + length, args, args_len);
    length += args_len;
    
    
    msg[0].addr = ihid->client->addr;
    msg[0].flags = 0; //ihid->client->flags & I2C_M_TEN;
    msg[0].len = length;
    msg[0].buf = cmd->data;
    
    if (data_len > 0) {
        msg[1].addr = ihid->client->addr;
        msg[1].flags = I2C_M_RD;
        msg[1].len = data_len;
        msg[1].buf = buf_recv;
        msg_num = 2;
        hid_device->reading = true;
    }
    
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)msg, msg_num);
    
    if (data_len > 0)
        hid_device->reading = false;
    
    if (ret != msg_num)
        return ret < 0 ? ret : -1;
    
    ret = 0;
    
    return ret;
}

int VoodooI2CHIDDevice::i2c_hid_set_power(i2c_hid *ihid, int power_state) {
    int ret;
    
    ret = __i2c_hid_command(ihid, &hid_set_power_cmd, 0, NULL, 0, NULL, 0, NULL);
    if (ret)
        IOLog("failed to change power settings \n");
    
    return ret;
}


void VoodooI2CHIDDevice::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    if (hid_device->reading)
        return;
    
    i2c_hid_get_input(ihid);
    
}

void VoodooI2CHIDDevice::i2c_hid_get_input(i2c_hid *ihid) {
    
};

bool VoodooI2CHIDDevice::i2c_hid_get_report_descriptor(i2c_hid *ihid){
    UInt rsize;
    unsigned char* rdesc;
    int ret;
    
    IOLog("reg: 0x%x\n",ihid->hdesc.wReportDescRegister);
    
    struct i2c_hid_cmd hid_report_desc_cmd = {
        .registerIndex = ihid->hdesc.wReportDescRegister,
        .opcode = 0x00,
        .length =2
    };
    
    rsize = (UInt16)(ihid->hdesc.wReportDescLength);
    if (!rsize || rsize > HID_MAX_DESCRIPTOR_SIZE){
        IOLog("%s::%s::Weird size of report descriptor (%u)\n", getName(), hid_device->name, rsize);
        return -1;
    }
    
    ret = i2c_hid_command(ihid, &hid_report_desc_cmd, rdesc, rsize);
    
    /*
    if (ret) {
        IOLog("%s::%s::Reading report descriptor failed", getName(), hid_device->name);
    }
    
    IOLog("%s::%s::Report descriptor: %*ph\n", getName(), hid_device->name, rdesc);
    
     
     */
    return 0;
    
};
