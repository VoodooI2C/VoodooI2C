#include "VoodooI2C.h"



#define super IOService
OSDefineMetaClassAndStructors(VoodooI2C, IOService);

// #define IGNORED_DEVICE "DLL05E3"
// #define IGNORED_DEVICE "SYNA7500"

/*
############################################################################################
############################################################################################
##### acpiConfigure
##### accepts I2CDevice* and get SSCN & FMCN from the ACPI tables data and set the
##### values in the struct depending on the configuration we want (will always be I2C master)
##### currently always returns true, TODO: return false when ACPI methods not available
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
 ##### accepts I2CBus* and enables the controller interrupt mask
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
 ##### accepts I2CDevice* and sets the enabled status of the device to bool enable
 ##### return true/false on success/failure
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

/*
 ############################################################################################
 ############################################################################################
 ##### funcI2C
 ##### accepts I2CBus* and returns the functionality of the bus instance
 ############################################################################################
 #############################################################################################
 */

UInt32 VoodooI2C::funcI2C(I2CBus* _dev) {
    return _dev->functionality;
}

/*
 ############################################################################################
 ############################################################################################
 ##### getACPIParams
 ##### accepts IOACPIPlatformDevice*, char*, UInt32 x3 and gets the method data using method, storing the results
 ##### in hcnt, lcnt
 ##### TODO: return true/false on success/failure
 ############################################################################################
 #############################################################################################
 */

void VoodooI2C::getACPIParams(IOACPIPlatformDevice* fACPIDevice, char* method, UInt32 *hcnt, UInt32 *lcnt, UInt32 *sda_hold) {
    OSObject *object;
    
    if(kIOReturnSuccess == fACPIDevice->evaluateObject(method, &object) && object) {
        OSArray* values = OSDynamicCast(OSArray, object);
        
        *hcnt = OSDynamicCast(OSNumber, values->getObject(0))->unsigned32BitValue();
        *lcnt = OSDynamicCast(OSNumber, values->getObject(1))->unsigned32BitValue();
        if(sda_hold)
            *sda_hold = OSDynamicCast(OSNumber, values->getObject(2))->unsigned32BitValue();
        
    }
    
    OSSafeReleaseNULL(object);
}

/*
 ############################################################################################
 ############################################################################################
 ##### getMatchedName
 ##### accepts IOService* and returns the name of the matched device
 ############################################################################################
 #############################################################################################
 */

char* VoodooI2C::getMatchedName(IOService* provider) {
    OSData *data;
    data = OSDynamicCast(OSData, provider->getProperty("name"));
    return (char*)data->getBytesNoCopy();
}

/*
 ############################################################################################
 ############################################################################################
 ##### handleTxAbortI2C
 ##### accepts I2CBus* and outputs transaction error in IOLog
 ##### TODO: change to void
 ############################################################################################
 #############################################################################################
 */

int VoodooI2C::handleTxAbortI2C(I2CBus* _dev) {
    
    IOLog("%s::%s::I2C Transaction error: 0x%08x - aborting\n", getName(), _dev->name, _dev->abort_source);
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
 ##### accepts I2CDevice* and initialises the I2C Bus. Returns true if successful.
 ############################################################################################
 #############################################################################################
 */

bool VoodooI2C::initI2CBus(I2CBus* _dev) {
    
    //read the bus type from DW_IC_COMP_TYPE
    UInt32 reg = readl(_dev, DW_IC_COMP_TYPE);
    
    
    //check if bus type is valid, TODO: port other possibility from linux drivers
    if (reg == DW_IC_COMP_TYPE_VALUE) {
        IOLog("%s::%s::Found valid Synopsys component, continuing with initialisation\n", getName(), _dev->name);
    } else {
        IOLog("%s::%s::Unknown Synopsys component type: 0x%08x\n", getName(), _dev->name, reg);
        return false;
    }
    
    enableI2CDevice(_dev, false);
    
    //write standard mode hcnt, lcnt to DW_IC_SS_SCL_XCNT
    writel(_dev, _dev->ss_hcnt, DW_IC_SS_SCL_HCNT);
    writel(_dev, _dev->ss_lcnt, DW_IC_SS_SCL_LCNT);
    IOLog("%s::%s::Standard-mode HCNT:LCNT = %d:%d\n", getName(), _dev->name, _dev->ss_hcnt, _dev->ss_lcnt);
    
    
    //write fast mode hcnt, lcnt to DW_IC_DS_SCL_XCNT
    writel(_dev, _dev->fs_hcnt, DW_IC_FS_SCL_HCNT);
    writel(_dev, _dev->fs_lcnt, DW_IC_FS_SCL_LCNT);
    IOLog("%s::%s::Fast-mode HCNT:LCNT = %d:%d\n", getName(), _dev->name, _dev->fs_hcnt, _dev->fs_lcnt);
    
    
    //check bus version in DW_IC_COMP_VERSION to see if we can adjust SDA hold time
    reg = readl(_dev, DW_IC_COMP_VERSION); 
    if  (reg >= DW_IC_SDA_HOLD_MIN_VERS)
        writel(_dev, _dev->sda_hold_time, DW_IC_SDA_HOLD);
    else
        IOLog("%s::%s::Warning: hardware too old to adjust SDA hold time\n", getName(), _dev->name);
    
    
    //write transit buffer depth in DW_IC_TX_TL
    writel(_dev, _dev->tx_fifo_depth - 1, DW_IC_TX_TL);
    
    //write receive buffer depth in DW_IC_RX_TL
    writel(_dev, 0, DW_IC_RX_TL);
    
    
    //set default i2c target address in DW_IC_TAR, TODO: remove this, don't think we need it
    writel(_dev, 0x2c, DW_IC_TAR);
    
    
    //write I2C master configuration to DW_IC_CON
    writel(_dev, _dev->master_cfg, DW_IC_CON);

    return true;
}

/*
 ############################################################################################
 ############################################################################################
 ##### mapI2CMemory
 ##### accepts I2CDevice* and maps the device memory and store the data in the struct
 ##### returns true if succesful
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

/*
 ############################################################################################
 ############################################################################################
 ##### readI2C
 ##### accepts I2CBus* and reads from the read buffer during an ongoing transaction
 ############################################################################################
 #############################################################################################
 */

void VoodooI2C::readI2C(I2CBus* _dev) {
    struct i2c_msg *msgs = _dev->msgs;
    int rx_valid;
    
    
    //loop over the read index (less than the total number of messages)
    for(; _dev->msg_read_idx < _dev->msgs_num; _dev->msg_read_idx++) {
        UInt32 len;
        UInt8 *buf;
        
        
        //if current message is not a read, skip
        if (!(msgs[_dev->msg_read_idx].flags & I2C_M_RD))
            continue;
        
        //if a read is not in progress then take the length and the current message in the loop
        //is a read then take the length and buffer from the current message, else just set the
        //length and buffer to the previous length and buffer
        if(!(_dev->status & STATUS_READ_IN_PROGRESS)) {
            len = msgs[_dev->msg_read_idx].len;
            buf = msgs[_dev->msg_read_idx].buf;
        } else {
            len = _dev->rx_buf_len;
            buf = _dev->rx_buf;
        }
        
        //check how many items left in receive buffer
        rx_valid = readl(_dev, DW_IC_RXFLR);
        
        //collect data from receive buffer
        for (; len > 0 && rx_valid > 0; len--, rx_valid--)
            *buf++ = readl(_dev, DW_IC_DATA_CMD);
            _dev->rx_outstanding--;
        
        //if there are still more messages to read, set status to read in progress and continue
        //else remove read in progress status
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
 ##### accepts I2CDevice* and int and reads register at offset from the device's memory map
 ##### returns register data
 ############################################################################################
 #############################################################################################
 */

UInt32 VoodooI2C::readl(I2CBus* _dev, int offset) {
    return *(const volatile UInt32 *)(_dev->mmio + offset);
}

/*
 ############################################################################################
 ############################################################################################
 ##### readClearIntrbitsI2C
 ##### accepts I2CBus* and determines which interrupt fires. Clears the relevant interrupt
 ##### and returns the interrupt type.
 ############################################################################################
 #############################################################################################
 */


UInt32 VoodooI2C::readClearIntrbitsI2C(I2CBus* _dev) {
    UInt32 stat;
    //get interrupt type from DW_IC_INTR_STAT
    stat = readl(_dev, DW_IC_INTR_STAT);
    
    //determine interrupt type and clear relevant interrupt
    if (stat & DW_IC_INTR_RX_UNDER)
        readl(_dev, DW_IC_CLR_RX_UNDER);
    if (stat & DW_IC_INTR_RX_OVER)
        readl(_dev, DW_IC_CLR_RX_OVER);
    if (stat & DW_IC_INTR_TX_OVER)
        readl(_dev, DW_IC_CLR_TX_OVER);
    if (stat & DW_IC_INTR_RD_REQ)
        readl(_dev, DW_IC_CLR_RD_REQ);
    if (stat & DW_IC_INTR_TX_ABRT) {
        
        //get transaction abort source from DW_IC_TX_ABRT_SOURCE
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

/*
 ############################################################################################
 ############################################################################################
 ##### setI2CPowerState
 ##### accepts I2CBus* and bool and sets physical power state in DSDT
 ############################################################################################
 #############################################################################################
 */



void VoodooI2C::setI2CPowerState(I2CBus* _dev, bool enabled) {
    _dev->provider->evaluateObject(enabled ? "_PS0" : "_PS3");
}

/*
 ############################################################################################
 ############################################################################################
 ##### setPowerState
 ##### accept unsigned long, IOService* and sets OS level power state
 ############################################################################################
 #############################################################################################
 */

IOReturn VoodooI2C::setPowerState(unsigned long powerState, IOService *whatDevice){
    
    // OS X going to sleep
    if (powerState == 0){
        
        //power off I2C bus
        setI2CPowerState(_dev, false);
        IOLog("%s::Going to Sleep!\n", getName());
        
    // OS X waking up
    } else {
        
        //power on I2C bus
        setI2CPowerState(_dev, true);
        
        
        //reinitialize I2C bus
        initI2CBus(_dev);
        
        //disable clock gating
        writel(_dev, 1, 0x800);
        
        //disable interrupts
        disableI2CInt(_dev);
        
        IOLog("%s::Woke up from Sleep!\n", getName());
    }
    return kIOPMAckImplied;
}

/*
 ############################################################################################
 ############################################################################################
 ##### start
 ##### accepts IOService* and initialises the driver and starts the bus
 ############################################################################################
 #############################################################################################
 */
bool VoodooI2C::start(IOService * provider) {
    
    IOLog("%s::Found I2C device %s\n", getName(), getMatchedName(provider));
    
    if(!super::start(provider))
        return false;
    
    //initialise OS power management
    PMinit();
    
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
    
    //set up workloop
    _dev->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!_dev->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    }
    
    _dev->workLoop->retain();
    
    //set up interrupts
    _dev->interruptSource =
    IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2C::interruptOccured), _dev->provider);
    
    if (_dev->workLoop->addEventSource(_dev->interruptSource) != kIOReturnSuccess) {
        IOLog("%s::%s::Could not add interrupt source to workloop\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    }
    
    _dev->interruptSource->enable();
    
    //set up command gate
    
    _dev->commandGate = IOCommandGate::commandGate(this);
    
    if (!_dev->commandGate || (_dev->workLoop->addEventSource(_dev->commandGate) != kIOReturnSuccess)) {
        IOLog("%s::%s::Failed to open command gate\n", getName(), _dev->name);
        return false;
    }
    
    //power on I2C bus
    
    setI2CPowerState(_dev, true);
    
    //map I2C bus memory
    
    if(!mapI2CMemory(_dev)) {
        IOLog("%s::%s::Failed to map memory\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    } else {
        setProperty("physical-address", (UInt32)_dev->mmap->getPhysicalAddress(), 32);
        setProperty("memory-length", (UInt32)_dev->mmap->getLength(), 32);
        setProperty("virtual-address", (UInt32)_dev->mmap->getVirtualAddress(), 32);
    }
    
    //set I2C bus default configuration options
    
    _dev->clk_rate_khz = 400;
    _dev->sda_hold_time = 0x12c;
    _dev->sda_falling_time = 300;
    _dev->scl_falling_time = 300;
    
    //define I2C bus functionality
    
    _dev->functionality = I2C_FUNC_I2C | I2C_FUNC_10BIT_ADDR| I2C_FUNC_SMBUS_BYTE | I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA | I2C_FUNC_SMBUS_I2C_BLOCK;
    
    //define I2C bus configuration
    
    _dev->master_cfg = DW_IC_CON_MASTER | DW_IC_CON_SLAVE_DISABLE | DW_IC_CON_RESTART_EN | DW_IC_CON_SPEED_FAST;
    
    //get ACPI configuration if they exist
    
    if(!acpiConfigure(_dev)) {
        IOLog("%s::%s::Failed to read ACPI config\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    }
    
    //initialise I2C bus
    
    if(!initI2CBus(_dev)) {
        IOLog("%s::%s::Failed to initialise I2C Bus\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    }
    
    //disable LPSS clock gating
    
    writel(_dev, 1, 0x800);
    
    //disable I2C interrupts
    
    disableI2CInt(_dev);
    
    
    
    registerService();
    
    IORegistryIterator* children;
    IORegistryEntry* child;
    
 
    //enumerate I2C children, TODO: major refactoring
    
    children = IORegistryIterator::iterateOver(_dev->provider, gIOACPIPlane);
    bus_devices_number = 0;
#warning "Begin crash with non-HID device"
    if (children != 0) {
        OSOrderedSet* set = children->iterateAll();
        if(set != 0) {
            OSIterator *iter = OSCollectionIterator::withCollection(set);
            if (iter != 0) {
                while( (child = (IORegistryEntry*)iter->getNextObject()) ) {
#ifdef IGNORED_DEVICE
                    if (strcmp((getMatchedName((IOService*)child)),(char*)IGNORED_DEVICE)){
#endif
                        if (strcmp(getMatchedName((IOService *)child), "CYAP0000") == 0)
                            bus_devices[bus_devices_number] = OSTypeAlloc(VoodooI2CCyapaGen3Device);
                        else if (strcmp(getMatchedName((IOService *)child), "ATML0001") == 0){
                            bus_devices[bus_devices_number] = OSTypeAlloc(VoodooI2CAtmelMxtScreenDevice);
                        } else if ((strcmp(getMatchedName((IOService *)child), "ELAN0000") == 0 ||
                                    strcmp(getMatchedName((IOService *)child), "ELAN0100") == 0 ||
                                    strcmp(getMatchedName((IOService *)child), "ELAN0600") == 0 ||
                                    strcmp(getMatchedName((IOService *)child), "ELAN1000") == 0)){
                                bus_devices[bus_devices_number] = OSTypeAlloc(VoodooI2CElanTouchpadDevice);
                        } else
                            bus_devices[bus_devices_number] = OSTypeAlloc(VoodooI2CHIDDevice);
                        if ( !bus_devices[bus_devices_number]               ||
                            !bus_devices[bus_devices_number]->init()       ||
                            !bus_devices[bus_devices_number]->attach(this, (IOService*)child) )
                        {
                            OSSafeReleaseNULL(bus_devices[bus_devices_number]);
                        } else {
                            bus_devices_number++;
                        }
#ifdef IGNORED_DEVICE
                    }
#endif
                }
                iter->release();
            }
            set->release();
        }
    }
    children->release();
#warning "End crash with non-HID device"
    
    
    

    // Declare an array of two IOPMPowerState structures (kMyNumberOfStates = 2).
    
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
    
    
    
    provider->joinPMtree(this);
    
    registerPowerDriver(this, myPowerStates, kMyNumberOfStates);
    
    return true;
     
     
}




/*
 ############################################################################################
 ############################################################################################
 ##### stop
 ##### accepts IOService* and stops the driver
 ############################################################################################
 #############################################################################################
 */

void VoodooI2C::stop(IOService * provider) {
    IOLog("%s::stop\n", getName());
    
    for(int i=0;i<=bus_devices_number;i++) {
        //bus_devices[i]->stop(this);
        OSSafeReleaseNULL(bus_devices[i]);
    }

    //stop the command gate
    if (_dev->commandGate) {
        _dev->workLoop->removeEventSource(_dev->commandGate);
        _dev->commandGate->release();
        _dev->commandGate = NULL;
    }
    
    //stop the interrupt source
    if (_dev->interruptSource) {
        _dev->workLoop->removeEventSource(_dev->interruptSource);
        _dev->interruptSource->disable();
        _dev->interruptSource = NULL;
    }
    
    //stop the work loop
    if (_dev->workLoop) {
        _dev->workLoop->release();
        _dev->workLoop = NULL;
    }
    
    //stop the bus
    if(_dev) {
        setI2CPowerState(_dev, false);

        _dev->provider->close(this);
        OSSafeReleaseNULL(_dev->provider);

        IOFree(_dev, sizeof(I2CBus));
    }
    
    //stop power management
    PMstop();
    
    super::stop(provider);
}

/*
 ############################################################################################
 ############################################################################################
 ##### waitBusNotBusyI2C
 ##### accepts I2CBus* and waits for the bus to become ready
 ############################################################################################
 #############################################################################################
 */
int VoodooI2C::waitBusNotBusyI2C(I2CBus* _dev) {
    int timeout = TIMEOUT * 100;
    
    while (readl(_dev, DW_IC_STATUS) & DW_IC_STATUS_ACTIVITY) {
        if (timeout <= 0) {
            IOLog("%s::%s::Warning: Timeout waiting for bus ready\n", getName(), _dev->name);
            return -1;
        }
        timeout--;
        
        IODelay(1100);
    }
    
    return 0;
}

/*
 ############################################################################################
 ############################################################################################
 ##### writel
 ##### accepts I2CBus*, UInt32 and int and writes to the register at the offset of the bus
 ##### memory
 ############################################################################################
 #############################################################################################
 */

void VoodooI2C::writel(I2CBus* _dev, UInt32 b, int offset) {
    *(volatile UInt32 *)(_dev->mmio + offset) = b;
}

/*
 ############################################################################################
 ############################################################################################
 ##### xferI2C
 ##### accepts I2CBus*, i2c_msg, int 
 ############################################################################################
 #############################################################################################
 */

int VoodooI2C::xferI2C(I2CBus* _dev, i2c_msg *msgs, int num) {
    int ret;
    AbsoluteTime abstime;
    IOReturn sleep;
    
//    IOLog("%s::%s::msgs: %d\n", getName(), _dev->name, num);
    
    
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
//        IOLog("%s::%s::Woken up\n", getName(), _dev->name);
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

int VoodooI2C::i2c_transfer(i2c_msg *msgs, int num) {
    I2CBus* phys = _dev;
    return phys->commandGate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooI2C::i2c_transfer_gated), phys, msgs, &num);
}

int VoodooI2C::i2c_transfer_gated(I2CBus* phys, i2c_msg *msgs, int *num) {
    
    
    int ret;
    
//    for (ret = 0 ; ret < *num; ret++ ) {
//        IOLog("master_xfer[%d] %s, addr=0x%02x, len=%d%s\n", ret, (msgs[ret].flags & I2C_M_RD) ? "R" : "W", msgs[ret].addr, msgs[ret].len, (msgs[ret].flags & I2C_M_RECV_LEN) ? "+" : "");
//    }
    
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

int VoodooI2C::i2c_master_recv(VoodooI2CHIDDevice::I2CDevice I2CDevice, UInt8 *buf, int count) {
    struct i2c_msg msg;
    int ret;
    
    msg.addr = I2CDevice.addr;
    msg.flags |= I2C_M_RD;
    msg.len = count;
    msg.buf = buf;
    
    ret = i2c_transfer(&msg, 1);
    
    if (ret) {
        IOLog("failed!\n");
    }
    
    return (ret == 1) ? count : ret;
    
}

int VoodooI2C::i2c_master_send(VoodooI2CHIDDevice::I2CDevice I2CDevice, UInt8 *buf, int count) {
    struct i2c_msg msg;
    int ret;
    
    msg.addr = I2CDevice.addr;
    msg.flags |= 0;
    msg.len = count;
    msg.buf = buf;
    
    ret = i2c_transfer(&msg, 1);
    
    if (ret) {
        IOLog("failed!\n");
    }
    
    return (ret == 1) ? count : ret;
    
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
                
        //IOLog("%s::%s::I2C transaction aborted with error 0x%x\n", getName(), _dev->name, _dev->abort_source);
        
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


void VoodooI2C::clearI2CInt(I2CBus* _dev) {
    readl(_dev, DW_IC_CLR_INTR);
}
