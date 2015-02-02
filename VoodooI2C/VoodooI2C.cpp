#include "VoodooI2C.h"



#define super IOService
OSDefineMetaClassAndStructors(VoodooI2C, IOService);

/*
############################################################################################
############################################################################################
##### acpiConfigure
##### accept I2CDevice* and get SSCN & FMCN from the ACPI tables data and set the
##### values in the struct depending on the configuration we want (will always be I2C master)
############################################################################################
#############################################################################################
 */

bool VoodooI2C::acpiConfigure(I2CBus* _dev) {
    
    _dev->tx_fifo_depth = 32;
    _dev->rx_fifo_depth = 32;
    
    getACPIParams(_dev->provider, (char*)"SSCN", &_dev->ss_hcnt, &_dev->ss_lcnt, NULL);
    getACPIParams(_dev->provider, (char*)"FMCN", &_dev->fs_hcnt, &_dev->fs_lcnt, &_dev->sda_hold_time);
    
    return true;
}

/*
 ############################################################################################
 ############################################################################################
 ##### disableI2CInt
 ##### **UNKNOWN**
 ############################################################################################
 #############################################################################################
 */

void VoodooI2C::disableI2CInt(I2CBus* _dev) {
    writel(_dev, 0, DW_IC_INTR_MASK);
}

/*
 ############################################################################################
 ############################################################################################
 ##### enableI2CDevice
 ##### accept I2CDevice* and sets the enabled status of the device to bool enable
 ############################################################################################
 #############################################################################################
 */

void VoodooI2C::enableI2CDevice(I2CBus* _dev, bool enable) {
    int timeout = 5000; //increased to compensate for missing usleep below
    
    do {
        writel(_dev, enable, DW_IC_ENABLE);
        
        if((readl(_dev, DW_IC_ENABLE_STATUS) & 1) == enable) {
            
            return;
        }
      
        
        //TODO: usleep(250) goes here, xcode is complaining about missing symbol though...
        IODelay(250);
        
        
    } while (timeout--);
    
    IOLog("Timedout waiting for device status to change!\n");
}

UInt32 VoodooI2C::funcI2C(I2CBus* _dev) {
    return _dev->functionality;
}

/*
 ############################################################################################
 ############################################################################################
 ##### getACPIParams
 ##### accept IOACPIPlatformDevice* and get the method data using method, storing the results
 ##### in hcnt, lcnt
 ############################################################################################
 #############################################################################################
 */

void VoodooI2C::getACPIParams(IOACPIPlatformDevice* fACPIDevice, char* method, UInt32 *hcnt, UInt32 *lcnt, UInt32 *sda_hold) {
    OSObject *object;
    
    if(kIOReturnSuccess == fACPIDevice->evaluateObject(method, &object) && object) {
        OSArray* values = OSDynamicCast(OSArray, object);
        
        //IOLog("ACPI Configuration: %s: 0x%02x\n", method, OSDynamicCast(OSNumber, values->getObject(0))->unsigned32BitValue());
        //IOLog("ACPI Configuration: %s: 0x%02x\n", method, OSDynamicCast(OSNumber, values->getObject(1))->unsigned32BitValue());
        //IOLog("ACPI Configuration: %s: 0x%02x\n", method, OSDynamicCast(OSNumber, values->getObject(2))->unsigned32BitValue());
        
        *hcnt = OSDynamicCast(OSNumber, values->getObject(0))->unsigned32BitValue();
        *lcnt = OSDynamicCast(OSNumber, values->getObject(1))->unsigned32BitValue();
        if(sda_hold)
            *sda_hold = OSDynamicCast(OSNumber, values->getObject(2))->unsigned32BitValue();
        
    }
    
    OSSafeReleaseNULL(object);
}

/*
VoodooI2C::I2CBus* VoodooI2C::getBusByName(char* name ) {
    if(strcmp(I2CBus1->name, name))
        return I2CBus1;
    else
        return I2CBus2;
    
    return 0;
}
 
*/
/*
 ############################################################################################
 ############################################################################################
 ##### getMatchedName
 ##### accept IOService* and get the name of the matched device
 ############################################################################################
 #############################################################################################
 */

char* VoodooI2C::getMatchedName(IOService* provider) {
    OSData *data;
    data = OSDynamicCast(OSData, provider->getProperty("name"));
    return (char*)data->getBytesNoCopy();
}

int VoodooI2C::handleTxAbortI2C(I2CBus* _dev) {
    
    IOLog("%s::%s::I2C Transaction error - aborting\n", getName(), _dev->name);
    return -1;
    
    //TODO: determine exactly what this function does...
    /*
    unsigned long abort_source = _dev->abort_source;
    int i;
    
    if (abort_source & DW_IC_TX_ABRT_NOACK) {
        f
    }
     */
}

/*
 ############################################################################################
 ############################################################################################
 ##### initI2CBus
 ##### accepts I2CDevice* and initialises the I2C Bus.
 ############################################################################################
 #############################################################################################
 */

bool VoodooI2C::initI2CBus(I2CBus* _dev) {
    
    UInt32 reg = readl(_dev, DW_IC_COMP_TYPE);
    
    if (reg == DW_IC_COMP_TYPE_VALUE) {
        IOLog("%s::%s::Found valid Synopsys component, continuing with initialisation\n", getName(), _dev->name);
    } else {
        IOLog("%s::%s::Unknown Synopsys component type: 0x%08x\n", getName(), _dev->name, reg);
        return false;
    }
    
    enableI2CDevice(_dev, false);
    
    //Linux drivers set up sda/scl falling times and hcnt, lcnt here but we already set it up from the ACPI
    writel(_dev, _dev->ss_hcnt, DW_IC_SS_SCL_HCNT);
    writel(_dev, _dev->ss_lcnt, DW_IC_SS_SCL_LCNT);
    IOLog("%s::%s::Standard-mode HCNT:LCNT = %d:%d\n", getName(), _dev->name, _dev->ss_hcnt, _dev->ss_lcnt);
    
    writel(_dev, _dev->fs_hcnt, DW_IC_FS_SCL_HCNT);
    writel(_dev, _dev->fs_lcnt, DW_IC_FS_SCL_LCNT);
    IOLog("%s::%s::Fast-mode HCNT:LCNT = %d:%d\n", getName(), _dev->name, _dev->fs_hcnt, _dev->fs_lcnt);
    
    reg = readl(_dev, DW_IC_COMP_VERSION); 
    if  (reg >= DW_IC_SDA_HOLD_MIN_VERS)
        writel(_dev, _dev->sda_hold_time, DW_IC_SDA_HOLD);
    else
        IOLog("%s::%s::Warning: hardware too old to adjust SDA hold time\n", getName(), _dev->name);
    
    writel(_dev, _dev->tx_fifo_depth - 1, DW_IC_TX_TL);
    writel(_dev, 0, DW_IC_RX_TL);
    
    writel(_dev, 0x2c, DW_IC_TAR);
    
    writel(_dev, _dev->master_cfg, DW_IC_CON);

    return true;
}

/*
 ############################################################################################
 ############################################################################################
 ##### mapI2CMemory
 ##### accept I2CDevice* and map the device memory and store the data in the struct
 ############################################################################################
 #############################################################################################
 */

bool VoodooI2C::mapI2CMemory(I2CBus* _dev) {
    
    if(_dev->provider->getDeviceMemoryCount()==0) {
        return false;
    } else {
        _dev->mmap = _dev->provider->mapDeviceMemoryWithIndex(0);
        if(!_dev->mmap) return false;
        _dev->mmio= _dev->mmap->getVirtualAddress();
        return true;
        
    }
    
}

void VoodooI2C::readI2C(I2CBus* _dev) {
    struct i2c_msg *msgs = _dev->msgs;
    int rx_valid;
    
    for(; _dev->msg_read_idx < _dev->msgs_num; _dev->msg_read_idx++) {
        UInt32 len;
        UInt8 *buf;
        
        if (!(msgs[_dev->msg_read_idx].flags & I2C_M_RD))
            continue;
        
        if(!(_dev->status & STATUS_READ_IN_PROGRESS)) {
            len = msgs[_dev->msg_read_idx].len;
            buf = msgs[_dev->msg_read_idx].buf;
        } else {
            len = _dev->rx_buf_len;
            buf = _dev->rx_buf;
        }
        
        rx_valid = readl(_dev, DW_IC_RXFLR);
        
        for (; len > 0 && rx_valid > 0; len--, rx_valid--)
            *buf++ = readl(_dev, DW_IC_DATA_CMD);
        
        if (len > 0) {
            _dev->status |= STATUS_READ_IN_PROGRESS;
            _dev->rx_buf_len = len;
            _dev->rx_buf = buf;
            return;
        } else
            _dev->status &= ~STATUS_READ_IN_PROGRESS;
    }
    
    }

/*
 ############################################################################################
 ############################################################################################
 ##### readl
 ##### accept I2CDevice* and read register at offset from the device's memory map
 ############################################################################################
 #############################################################################################
 */

UInt32 VoodooI2C::readl(I2CBus* _dev, int offset) {
    return *(const volatile UInt32 *)(_dev->mmio + offset);
}

UInt32 VoodooI2C::readClearIntrbitsI2C(I2CBus* _dev) {
    UInt32 stat;
    
    stat = readl(_dev, DW_IC_INTR_STAT);
    
    if (stat & DW_IC_INTR_RX_UNDER)
        readl(_dev, DW_IC_CLR_RX_UNDER);
    if (stat & DW_IC_INTR_RX_OVER)
        readl(_dev, DW_IC_CLR_RX_OVER);
    if (stat & DW_IC_INTR_TX_OVER)
        readl(_dev, DW_IC_CLR_TX_OVER);
    if (stat & DW_IC_INTR_RD_REQ)
        readl(_dev, DW_IC_CLR_RD_REQ);
    if (stat & DW_IC_INTR_TX_ABRT) {
        _dev->abort_source = readl(_dev, DW_IC_TX_ABRT_SOURCE);
        readl(_dev, DW_IC_CLR_TX_ABRT);
    }
    if (stat & DW_IC_INTR_RX_DONE)
        readl(_dev, DW_IC_CLR_RX_DONE);
    if (stat & DW_IC_INTR_ACTIVITY)
        readl(_dev, DW_IC_CLR_ACTIVITY);
    if (stat & DW_IC_INTR_STOP_DET)
        readl(_dev, DW_IC_CLR_STOP_DET);
    if (stat & DW_IC_INTR_START_DET)
        readl(_dev, DW_IC_CLR_START_DET);
    if (stat & DW_IC_INTR_GEN_CALL)
        readl(_dev, DW_IC_CLR_GEN_CALL);
    
    return stat;
}


void VoodooI2C::setI2CPowerState(I2CBus* _dev, bool enabled) {
    _dev->provider->evaluateObject(enabled ? "_PS0" : "_PS3");
}

/*
 ############################################################################################
 ############################################################################################
 ##### start
 ##### accept IOService* and start the driver
 ############################################################################################
 #############################################################################################
 */

bool VoodooI2C::start(IOService * provider) {
    
    IOLog("%s::Found I2C device %s\n", getName(), getMatchedName(provider));
    
    if(!super::start(provider))
        return false;
    
    fACPIDevice = OSDynamicCast(IOACPIPlatformDevice, provider);
    
    if (!fACPIDevice)
        return false;
    
    _dev = (I2CBus *)IOMalloc(sizeof(I2CBus));
        
    
    _dev->provider = fACPIDevice;
    _dev->name = getMatchedName(fACPIDevice);
    
    _dev->provider->retain();
    
    if(!_dev->provider->open(this)) {
        IOLog("%s::%s::Failed to open ACPI device\n", getName(), _dev->name);
        return false;
    }
    
    
    _dev->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!_dev->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    }
    
    _dev->workLoop->retain();
    
    
    _dev->interruptSource =
    IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2C::interruptOccured), _dev->provider);
    
    if (_dev->workLoop->addEventSource(_dev->interruptSource) != kIOReturnSuccess) {
        IOLog("%s::%s::Could not add interrupt source to workloop\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    }
    
    _dev->interruptSource->enable();
    
    _dev->commandGate = IOCommandGate::commandGate(this);
    
    if (!_dev->commandGate || (_dev->workLoop->addEventSource(_dev->commandGate) != kIOReturnSuccess)) {
        IOLog("%s::%s::Failed to open command gate\n", getName(), _dev->name);
        return false;
    }
    
    setI2CPowerState(_dev, true);
    
    if(!mapI2CMemory(_dev)) {
        IOLog("%s::%s::Failed to map memory\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    } else {
        setProperty("physical-address", (UInt32)_dev->mmap->getPhysicalAddress(), 32);
        setProperty("memory-length", (UInt32)_dev->mmap->getLength(), 32);
        setProperty("virtual-address", (UInt32)_dev->mmap->getVirtualAddress(), 32);
    }
    
    _dev->clk_rate_khz = 400;
    _dev->sda_hold_time = 0x12c;//_dev->clk_rate_khz*300 + 500000;
    _dev->sda_falling_time = 300;
    _dev->scl_falling_time = 300;
    
    //UInt32 param1 = readl(_dev, DW_IC_COMP_PARAM_1);
    
    //_dev->tx_fifo_depth = ((param1 >> 16) & 0xff) + 1;
    //_dev->rx_fifo_depth = ((param1 >> 8) & 0xff) + 1;
    
    _dev->functionality = I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR| I2C_FUNC_SMBUS_BYTE | I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA | I2C_FUNC_SMBUS_I2C_BLOCK;
    
    _dev->master_cfg = DW_IC_CON_MASTER | DW_IC_CON_SLAVE_DISABLE | DW_IC_CON_RESTART_EN | DW_IC_CON_SPEED_FAST;
    
    if(!acpiConfigure(_dev)) {
        IOLog("%s::%s::Failed to read ACPI config\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    }
    
    if(!initI2CBus(_dev)) {
        IOLog("%s::%s::Failed to initialise I2C Bus\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    }
    
    writel(_dev, 1, 0x800);
    
    disableI2CInt(_dev);
    
    
    
    //TODO: Interrupt stuff
    
    registerService();
    
    IORegistryIterator* children;
    IORegistryEntry* child;
    
    //hid_device = (I2CDevice *)IOMalloc(sizeof(I2CDevice));
    

    //hid_device->addr = 0x002c;
    //hid_device->_dev = _dev;
    
    
    children = IORegistryIterator::iterateOver(_dev->provider, gIOACPIPlane);
    if (children != 0) {
        OSOrderedSet* set = children->iterateAll();
        if(set != 0) {
            OSIterator *iter = OSCollectionIterator::withCollection(set);
            if (iter != 0) {
                while( (child = (IORegistryEntry*)iter->getNextObject()) ) {
                    //if (!strcmp((getMatchedName((IOService*)child)),(char*)"SYNA7500")){
                        bus_devices[bus_devices_number] = OSTypeAlloc(VoodooI2CHIDDevice);
                        if ( !bus_devices[bus_devices_number]               ||
                            !bus_devices[bus_devices_number]->init()       ||
                            !bus_devices[bus_devices_number]->attach(this, (IOService*)child) )
                        {
                            OSSafeReleaseNULL(bus_devices[bus_devices_number]);
                        }
                    //}
                }
                iter->release();
            }
            set->release();
        }
    }
    children->release();
    
    
    //if (initHIDDevice(hid_device))
    //    IOLog("%s::%s::Failed to initialise HID Device\n", getName(), _dev->name);
    
    
    return true;
     
     
}

int VoodooI2C::initHIDDevice(I2CDevice *hid_device) {
    int ret;
    UInt16 hidRegister;
    
    IOLog("HID Probe called for i2c 0x%02x\n", hid_device->addr);
    
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
    
    //ret = i2c_hid_set_power(ihid, I2C_HID_PWR_ON);
    //if(ret<0)
    //    goto err;
    
    ret = i2c_hid_fetch_hid_descriptor(ihid);
    if (ret < 0)
        goto err;

    /*
    hid_device->interruptSource =
    IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2C::HIDInterruptOccured), hid_device->provider);
    
    if (_dev->workLoop->addEventSource(hid_device->interruptSource) != kIOReturnSuccess) {
        IOLog("%s::%s::Could not add interrupt source to workloop\n", getName(), _dev->name);
        return -1;
    }
    
   hid_device->interruptSource->enable();
    
    
    
   hid_device->commandGate = IOCommandGate::commandGate(this);
    
    if (!hid_device->commandGate || (_dev->workLoop->addEventSource(hid_device->commandGate) != kIOReturnSuccess)) {
        IOLog("%s::%s::Failed to open HID command gate\n", getName(), _dev->name);
        return -1;
    }
     */

    
    return 0;
    
err:
    i2c_hid_free_buffers(ihid, HID_MIN_BUFFER_SIZE);
    IOFree(ihid, sizeof(i2c_hid));
    return ret;
}

int VoodooI2C::i2c_hid_acpi_pdata(i2c_hid *ihid) {
    //static UInt8 i2c_hid_guid[] = {
   //     0xF7, 0xF6, 0xDF, 0x3C, 0x67, 0x42, 0x55, 0x45,
    //    0xAD, 0x05, 0xB3, 0x0A, 0x3D, 0x89, 0x38, 0xDE,
   // };
    
    ihid->pdata.hid_descriptor_address = 0x20;//ihid->client->provider->evaluateInteger("_DSM", &result);

    return 0;
}

int VoodooI2C::i2c_hid_alloc_buffers(i2c_hid *ihid, UInt report_size) {
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

void VoodooI2C::i2c_hid_free_buffers(i2c_hid *ihid, UInt report_size) {
    IOFree(ihid->inbuf, report_size);
    IOFree(ihid->argsbuf, report_size);
    IOFree(ihid->cmdbuf, sizeof(UInt8) + sizeof(UInt16) + sizeof(UInt16) + report_size);
    ihid->inbuf = NULL;
    ihid->cmdbuf = NULL;
    ihid->argsbuf = NULL;
    ihid->bufsize = 0;
}

int VoodooI2C::i2c_hid_fetch_hid_descriptor(i2c_hid *ihid) {
    struct i2c_hid_desc *hdesc = &ihid->hdesc;
    UInt dsize;
    int ret;
    
    ret = i2c_hid_command(ihid, &hid_descr_cmd, ihid->hdesc_buffer, sizeof(struct i2c_hid_desc));
    
    if (ret)
    {
        IOLog("%s::%s::hid_descr_cmd failed\n", getName(), _dev->name);
        return -1;
    }
    
    if((UInt16)(hdesc->bcdVersion) != 0x0100) {
        IOLog("%s::%s::Unexpected HID descriptor bcdVersion %x\n", getName(), _dev->name, (UInt16)(hdesc->bcdVersion));
        return -1;
    }
    
    //dsize = (UInt16)(hdesc->wHIDDescLength);
    
    //if (dsize != sizeof(struct i2c_hid_desc)) {
    //    IOLog("%s::%s::weird size of HID descriptor\n", getName(), _dev->name);
    //    return -1;
    //}
    
    return 0;
}

int VoodooI2C::i2c_hid_command(i2c_hid *ihid, struct i2c_hid_cmd *command, unsigned char *buf_recv, int data_len) {
    return __i2c_hid_command(ihid, command, 0, 0, NULL, 0, buf_recv, data_len);
}

int VoodooI2C::__i2c_hid_command(i2c_hid *ihid, struct i2c_hid_cmd *command, UInt8 reportID, UInt8 reportType, UInt8 *args, int args_len, unsigned char *buf_recv, int data_len) {
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
        //set_bit(I2C_HID_READ_PENDING, &ihid->flags);
    }
    
    ret = i2c_transfer(ihid->client->_dev, msg, msg_num);
    
    if (ret != msg_num)
        return ret < 0 ? ret : -1;
    
    ret = 0;
    
    return ret;
}

int VoodooI2C::i2c_hid_set_power(i2c_hid *ihid, int power_state) {
    int ret;
    
    ret = __i2c_hid_command(ihid, &hid_set_power_cmd, 0, NULL, 0, NULL, 0, NULL);
    if (ret)
        IOLog("failed to change power settings \n");
    
    return ret;
}
/*
 ############################################################################################
 ############################################################################################
 ##### stop
 ##### accept IOService* and stop the driver
 ############################################################################################
 #############################################################################################
 */

void VoodooI2C::stop(IOService * provider) {
    IOLog("%s::stop\n", getName());
    
    for(int i=0;i<=bus_devices_number;i++) {
        OSSafeReleaseNULL(bus_devices[i]);
    }
    
    /*
        
    _dev->workLoop->removeEventSource(hid_device->interruptSource);
    hid_device->commandGate->release();
    hid_device->commandGate = NULL;
     
     
    
    _dev->workLoop->removeEventSource(hid_device->interruptSource);
    hid_device->interruptSource->disable();
    hid_device->interruptSource = NULL;
     
     */

     
    
    //i2c_hid_free_buffers(ihid, HID_MIN_BUFFER_SIZE);
    
    _dev->workLoop->removeEventSource(_dev->commandGate);
    _dev->commandGate->release();
    _dev->commandGate = NULL;
    
    _dev->workLoop->removeEventSource(_dev->interruptSource);
    _dev->interruptSource->disable();
    _dev->interruptSource = NULL;
    
    _dev->workLoop->release();
    _dev->workLoop = NULL;
    
    setI2CPowerState(_dev, false);

    _dev->provider->close(this);
    OSSafeReleaseNULL(_dev->provider);

    IOFree(hid_device, sizeof(I2CDevice*));
    IOFree(_dev, sizeof(I2CBus));
    
    
    super::stop(provider);
}

int VoodooI2C::waitBusNotBusyI2C(I2CBus* _dev) {
    int timeout = TIMEOUT * 100;
    
    while (readl(_dev, DW_IC_STATUS) & DW_IC_STATUS_ACTIVITY) {
        if (timeout <= 0) {
            IOLog("%s::%s::Warning: Timeout waiting for bus ready\n", getName(), _dev->name);
            return -1;
        }
        timeout--;
        
        IODelay(1100);
        //TODO: usleep(1100) goes here
    }
    
    return 0;
}

void VoodooI2C::writel(I2CBus* _dev, UInt32 b, int offset) {
    *(volatile UInt32 *)(_dev->mmio + offset) = b;
}

int VoodooI2C::xferI2C(I2CBus* _dev, i2c_msg *msgs, int num) {
    int ret;
    AbsoluteTime abstime;
    IOReturn sleep;
    
    IOLog("%s::%s::msgs: %d\n", getName(), _dev->name, num);
    
    
    _dev->msgs = msgs;
    _dev->msgs_num = num;
    _dev->cmd_err = 0;
    _dev->msg_write_idx = 0;
    _dev->msg_read_idx = 0;
    _dev->msg_err = 0;
    _dev->status = STATUS_IDLE;
    _dev->abort_source = 0;
    _dev->rx_outstanding = 0;
    
    ret = waitBusNotBusyI2C(_dev);
    
    if (ret<0)
        goto done;
    
    xferInitI2C(_dev);
    
    nanoseconds_to_absolutetime(10000, &abstime);
    
    sleep = _dev->commandGate->commandSleep(&_dev->commandComplete, abstime);
                                            
    if ( sleep == THREAD_TIMED_OUT ) {
        IOLog("%s::%s::Warning: Timeout waiting for bus ready\n", getName(), _dev->name);
        initI2CBus(_dev);
        ret = -1;
        goto done;
    } else {
        IOLog("%s::%s::Woken up\n", getName(), _dev->name);
    }
    
    enableI2CDevice(_dev, false);
    
    
    if (_dev->msg_err) {
        ret = _dev->msg_err;
        goto done;
    }
    
    if(!_dev->cmd_err) {
        ret = num;
        goto done;
    }
    
    if(_dev->cmd_err == DW_IC_ERR_TX_ABRT) {
        ret = handleTxAbortI2C(_dev);
        goto done;
    }
    
    ret = -EAGAIN;
    
    goto done;
    
    
done:
    
    return ret;
}

void VoodooI2C::xferInitI2C(I2CBus* _dev) {
    struct i2c_msg *msgs = _dev->msgs;
    
    enableI2CDevice(_dev, 0);
    writel(_dev, msgs[_dev->msg_write_idx].addr, DW_IC_TAR);
    
    disableI2CInt(_dev);
    
    enableI2CDevice(_dev, 1);
    clearI2CInt(_dev);
    
    writel(_dev, DW_IC_INTR_DEFAULT_MASK, DW_IC_INTR_MASK);
}

void VoodooI2C::registerDump(I2CBus* _dev) {
    IOLog("DW_IC_CON: 0x%x\n", readl(_dev, DW_IC_CON));
    IOLog("DW_IC_TAR: 0x%x\n", readl(_dev, DW_IC_TAR));
    IOLog("DW_IC_SAR: 0x%x\n", readl(_dev, DW_IC_SAR));
    //IOLog("DW_IC_DATA_CMD: 0x%x\n", readl(_dev, DW_IC_DATA_CMD));
    IOLog("DW_IC_SS_SCL_HCNT: 0x%x\n", readl(_dev, DW_IC_SS_SCL_HCNT));
    IOLog("DW_IC_SS_SCL_LCNT: 0x%x\n", readl(_dev, DW_IC_SS_SCL_LCNT));
    IOLog("DW_IC_FS_SCL_HCNT: 0x%x\n", readl(_dev, DW_IC_FS_SCL_HCNT));
    IOLog("DW_IC_FS_SCL_LCNT: 0x%x\n", readl(_dev, DW_IC_FS_SCL_LCNT));
    IOLog("DW_IC_INTR_STAT: 0x%x\n", readl(_dev, DW_IC_INTR_STAT));
    IOLog("DW_IC_INTR_MASK: 0x%x\n", readl(_dev, DW_IC_FS_SCL_LCNT));
    IOLog("DW_IC_RAW_INTR_STAT: 0x%x\n", readl(_dev, DW_IC_RAW_INTR_STAT));
    IOLog("DW_IC_RX_TL: 0x%x\n", readl(_dev, DW_IC_RX_TL));
    IOLog("DW_IC_TX_TL: 0x%x\n", readl(_dev, DW_IC_TX_TL));
    IOLog("DW_IC_STATUS: 0x%x\n", readl(_dev, DW_IC_STATUS));
    IOLog("DW_IC_TXFLR: 0x%x\n", readl(_dev, DW_IC_TXFLR));
    IOLog("DW_IC_RXFLR: 0x%x\n", readl(_dev, DW_IC_RXFLR));
    IOLog("DW_IC_SDA_HOLD: 0x%x\n", readl(_dev, DW_IC_SDA_HOLD));
    IOLog("DW_IC_TX_ABRT_SOURCE: 0x%x\n", readl(_dev, DW_IC_TX_ABRT_SOURCE));
    IOLog("DW_IC_DMA_CR: 0x%x\n", readl(_dev, DW_IC_DMA_CR));
    IOLog("DW_IC_DMA_TDLR: 0x%x\n", readl(_dev, DW_IC_DMA_TDLR));
    IOLog("DW_IC_DMA_RDLR: 0x%x\n", readl(_dev, DW_IC_DMA_RDLR));
    IOLog("DW_IC_SDA_SETUP: 0x%x\n", readl(_dev, DW_IC_SDA_SETUP));
    IOLog("DW_IC_ENABLE_STATUS: 0x%x\n", readl(_dev, DW_IC_ENABLE_STATUS));
    IOLog("DW_IC_FS_SPKLEN: 0x%x\n", readl(_dev, DW_IC_FS_SPKLEN));
    IOLog("DW_IC_COMP_PARAM_1: 0x%x\n", readl(_dev, DW_IC_COMP_PARAM_1));
    IOLog("DW_IC_COMP_VERSION: 0x%x\n", readl(_dev, DW_IC_COMP_VERSION));
    IOLog("DW_IC_COMP_TYPE: 0x%x\n", readl(_dev, DW_IC_COMP_TYPE));
    IOLog("privatespace: 0x%x\n", readl(_dev, 0x800));

}

void VoodooI2C::xferMsgI2C(I2CBus* _dev) {
    struct i2c_msg *msgs = _dev->msgs;
    UInt32 intr_mask;
    int tx_limit, rx_limit;
    UInt32 addr = msgs[_dev->msg_write_idx].addr;
    UInt32 buf_len = _dev->tx_buf_len;
    UInt8 *buf = _dev->tx_buf;
    bool need_restart = false;
    
    intr_mask = DW_IC_INTR_DEFAULT_MASK;
    
    for (; _dev->msg_write_idx < _dev->msgs_num; _dev->msg_write_idx++) {
        
        if (msgs[_dev->msg_write_idx].addr != addr) {
            _dev->msg_err = -1;
            break;
        }
        
        if (msgs[_dev->msg_write_idx].len == 0) {
            _dev->msg_err = -1;
            break;
        }
        
        if (!(_dev->status & STATUS_WRITE_IN_PROGRESS)) {
            buf = msgs[_dev->msg_write_idx].buf;
            buf_len = msgs[_dev->msg_write_idx].len;
            
            if ((_dev->master_cfg & DW_IC_CON_RESTART_EN) && (_dev->msg_write_idx > 0)) {
                need_restart = true;
            }
        }
        
        tx_limit = _dev->tx_fifo_depth - readl(_dev, DW_IC_TXFLR);
        rx_limit = _dev->rx_fifo_depth - readl(_dev, DW_IC_RXFLR);
        
        
        while (buf_len > 0 && tx_limit > 0 && rx_limit > 0) {
            

            UInt32 cmd = 0;
            
            if (_dev->msg_write_idx == _dev->msgs_num - 1 && buf_len == 1) {
                cmd |= 0x200;
            }
            
            if (need_restart) {
                cmd |= 0x400;
                need_restart = false;
            }
            if (msgs[_dev->msg_write_idx].flags & I2C_M_RD) {
                if (rx_limit - _dev->rx_outstanding <= 0) {
                    break;
                }
                writel(_dev, cmd | 0x100, DW_IC_DATA_CMD);
                rx_limit--;
                _dev->rx_outstanding++;
            } else {
                writel(_dev, cmd | *buf++, DW_IC_DATA_CMD);
            }
            tx_limit--; buf_len--;
        }
        
        _dev->tx_buf = buf;
        _dev->tx_buf_len = buf_len;
        
        if (buf_len > 0) {
            _dev->status |= STATUS_WRITE_IN_PROGRESS;
            break;
        } else {
            _dev->status &= ~STATUS_WRITE_IN_PROGRESS;
        }
        
    }
   
    if (_dev->msg_write_idx == _dev->msgs_num) {
        intr_mask &= ~DW_IC_INTR_TX_EMPTY;
    }
    
    if (_dev->msg_err) {
        intr_mask = 0;
    }
    
    writel(_dev, intr_mask, DW_IC_INTR_MASK);
    
    //registerDump(_dev);

    //readl(_dev, DW_IC_INTR_MASK);
    
    //_dev->commandGate->commandWakeup(&_dev->commandComplete);
    
}

int VoodooI2C::i2c_transfer(I2CBus* phys, i2c_msg *msgs, int num) {
    
    return phys->commandGate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooI2C::i2c_transfer_gated), phys, msgs, &num);
}

int VoodooI2C::i2c_transfer_gated(I2CBus* phys, i2c_msg *msgs, int *num) {
    
    int ret;
    
    for (ret = 0 ; ret < *num; ret++ ) {
        IOLog("master_xfer[%d] %s, addr=0x%02x, len=%d%s\n", ret, (msgs[ret].flags & I2C_M_RD) ? "R" : "W", msgs[ret].addr, msgs[ret].len, (msgs[ret].flags & I2C_M_RECV_LEN) ? "+" : "");
    }
    
    ret = __i2c_transfer(phys, msgs, *num);

    
    return ret;
    
}

int VoodooI2C::__i2c_transfer(I2CBus* phys, i2c_msg *msgs, int num) {

    int ret, tries;
    
    for (ret = 0, tries = 0; tries <= 5; tries++) {
        ret = xferI2C(phys, msgs, num);
        if (ret != -EAGAIN)
            break;
    }
    
    return ret;
    
}

void VoodooI2C::interruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount) {
    UInt32 stat, enabled;
    
    enabled = readl(_dev, DW_IC_ENABLE);
    stat = readl(_dev, DW_IC_RAW_INTR_STAT);
    
    if (!enabled || !(stat &~DW_IC_INTR_ACTIVITY))
        return;
    
    
    stat = readClearIntrbitsI2C(_dev);


    if (stat & DW_IC_INTR_TX_ABRT) {
        //IOLog("%s::%s::Interrupt Aborting transaction\n", getName(), _dev->name);

        _dev->cmd_err |= DW_IC_ERR_TX_ABRT;
        _dev->status = STATUS_IDLE;
        
        writel(_dev, 0, DW_IC_INTR_MASK);
        goto tx_aborted;
    }
    
    if (stat & DW_IC_INTR_RX_FULL) {
        //IOLog("%s::%s::interrupt reading transaction\n", getName(), _dev->name);
        readI2C(_dev);
    }
    
    if (stat & DW_IC_INTR_TX_EMPTY) {
        //IOLog("%s::%s::interrupt xfer transaction\n", getName(), _dev->name);
        xferMsgI2C(_dev);
    }
    
tx_aborted:
    if ((stat & (DW_IC_INTR_TX_ABRT | DW_IC_INTR_STOP_DET)) || _dev->msg_err) {
        _dev->commandGate->commandWakeup(&_dev->commandComplete);
    }
}

void VoodooI2C::HIDInterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    IOLog("%s::%s::RMI4 Interrupt occured\n", getName(), _dev->name);

}


void VoodooI2C::clearI2CInt(I2CBus* _dev) {
    readl(_dev, DW_IC_CLR_INTR);
}
