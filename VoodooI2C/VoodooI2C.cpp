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
    
    bool fs_mode = _dev->master_cfg & DW_IC_CON_SPEED_FAST;
    
    _dev->tx_fifo_depth = 32;
    _dev->rx_fifo_depth = 32;
    
    getACPIParams(_dev->provider, (char*)"SSCN", &_dev->ss_hcnt, &_dev->ss_lcnt, fs_mode ? NULL : &_dev->sda_hold_time);
    getACPIParams(_dev->provider, (char*)"FMCN", &_dev->fs_hcnt, &_dev->fs_lcnt, fs_mode ? &_dev->sda_hold_time : NULL);
    
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
    int timeout = 500; //increased to compensate for missing usleep below
    
    do {
        writel(_dev, enable, DW_IC_ENABLE);
        
        if((readl(_dev, DW_IC_ENABLE_STATUS) & 1) == enable) {
            
            return;
        }
      
        
        //TODO: usleep(250) goes here, xcode is complaining about missing symbol though...
        
        
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
    
    _dev->commandGate->commandWakeup(&_dev->commandComplete);
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
    _dev->sda_hold_time = _dev->clk_rate_khz*300 + 500000;
    _dev->sda_falling_time = 300;
    _dev->scl_falling_time = 300;
    
    _dev->functionality = I2C_FUNC_I2C;
    _dev->master_cfg = DW_IC_CON_MASTER;
    
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
    
    disableI2CInt(_dev);
    
    
    
    //TODO: Interrupt stuff
    
    registerService();
    
    IORegistryIterator* children;
    IORegistryEntry* child;
    
    _rmidev = (RMI4Device *)IOMalloc(sizeof(RMI4Device));
    
    rmi_i2c_data data;
    
    _rmidev->addr = 0x2c;
    _rmidev->name = (char*)"Synaptics Clearpad 7500";
    _rmidev->_dev = _dev;
    _rmidev->data = &data;
    
    
    children = IORegistryIterator::iterateOver(_dev->provider, gIOACPIPlane);
    if (children != 0) {
        OSOrderedSet* set = children->iterateAll();
        if(set != 0) {
            OSIterator *iter = OSCollectionIterator::withCollection(set);
            if (iter != 0) {
                while( (child = (IORegistryEntry*)iter->getNextObject()) ) {
                        child->attachToParent(this, gIOServicePlane);
                    if (!strcmp((getMatchedName((IOService*)child)),(char*)"SYNA7500")){
                        _rmidev->provider = OSDynamicCast(IOACPIPlatformDevice, child);
                    }
                }
                iter->release();
            }
            set->release();
        }
    }
    children->release();
    
    if(initRMI4Device(_rmidev)) {
        IOLog("%s::%s::Failed to initialise RMI4 Device\n", getName(), _dev->name);
        VoodooI2C::stop(provider);
        return false;
    }
    
    
    //if(probeRMI4Device(_rmidev)) {
    //    IOLog("%s::%s::Failed to probe RMI4 Device\n", getName(), _dev->name);
    //}
    
    //if(rmi_driver_f01_init(_rmidev)) {
    //    IOLog("%s::%s::Couldn't initialise f01\n", getName(), _dev->name);
    //} else
    //    IOLog("initialised successfully!\n");
    
    
    return true;
     
     
}

int VoodooI2C::rmi_driver_f01_init(RMI4Device *rmi_dev) {
    struct rmi_driver_data *data = rmi_dev->driver_data;
    int error;
    UInt8 temp;
    
    IOLog("test %x!!\n", data->f01_fd.control_base_addr);
    return -1;
    
    error = rmi_i2c_read(rmi_dev, data->f01_fd.control_base_addr, &temp);
    if (error < 0)
        return -1;
    
    return 0;
}

int VoodooI2C::probeRMI4Device(RMI4Device *phys) {
    struct rmi_driver_data *data;
    struct pdt_entry {
        UInt8 query_base_addr;
        UInt8 command_base_addr;
        UInt8 control_base_addr;
        UInt8 data_base_addr;
        UInt8 interrupt_source_count;
        UInt8 function_number;
    } pdt_entry;
    
    struct rmi_function_handler *fh;
    struct rmi_function_container *fc;
    int error;
    int i;
    int page;
    int irq_count = 0;
    bool done = false;
    
    for (page=0; (page <= RMI4_MAX_PAGE) && !done; page++) {
        UInt16 page_start = RMI4_PAGE_SIZE * page;
        UInt16 pdt_start = page_start + PDT_START_SCAN_LOCATION;
        UInt16 pdt_end = page_start + PDT_END_SCAN_LOCATION;
        
        done = true;
        
        for (i = pdt_start; i >= pdt_end; i -= sizeof(pdt_entry)) {
            error = rmi_i2c_read_block(phys, i, (UInt8 *)&pdt_entry, sizeof(pdt_entry));
            
            if (error != sizeof(pdt_entry)) {
                goto err_free_data;
            }
            
            if (RMI4_END_OF_PDT(pdt_entry.function_number)) {
                break;
            }
            
            
            done = false;
            
            phys->driver_data = data;
            
           
            
            if (pdt_entry.function_number) {
                IOLog("got ib!\n");
                data->f01_fd.query_base_addr = pdt_entry.query_base_addr + page_start;
                data->f01_fd.command_base_addr = pdt_entry.command_base_addr + page_start;
                data->f01_fd.control_base_addr = pdt_entry.control_base_addr + page_start;
                data->f01_fd.data_base_addr = pdt_entry.data_base_addr + page_start;
                data->f01_fd.function_number = pdt_entry.function_number;
                data->f01_fd.interrupt_source_count = pdt_entry.interrupt_source_count;
                data->f01_num_of_irqs = pdt_entry.interrupt_source_count & 0x07;
                data->f01_irq_pos = irq_count;
                
                irq_count += data->f01_num_of_irqs;
                
                continue;
            }
            
            fc = (struct rmi_function_container*)IOMalloc(sizeof(struct rmi_function_container));
            
            if (!fc) {
                IOLog("not enough memory!!\n");
                return -1;
            }
            
            fc->fd.query_base_addr = pdt_entry.query_base_addr + page_start;
            fc->fd.command_base_addr = pdt_entry.command_base_addr + page_start;
            fc->fd.control_base_addr = pdt_entry.control_base_addr + page_start;
            fc->fd.data_base_addr = pdt_entry.data_base_addr + page_start;
            fc->fd.function_number = pdt_entry.function_number;
            fc->fd.interrupt_source_count = pdt_entry.interrupt_source_count;
            
            //fc->rmi_dev = phys;
            fc->num_of_irqs = pdt_entry.interrupt_source_count & 0x07;
            fc->irq_pos = irq_count;
            irq_count += fc->num_of_irqs;
        }
        
    }
    
    return 0;
    
err_free_data:
    return -1;
}

int VoodooI2C::initRMI4Device(RMI4Device* _rmidev) {
    int error = rmi_set_page(_rmidev, 0);
    if (error < 0) {
        IOLog("error setting page\n");
        return -1;
    }
    

    _rmidev->interruptSource =
    IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2C::RMI4InterruptOccured), _rmidev->provider);
    
    if (_dev->workLoop->addEventSource(_rmidev->interruptSource) != kIOReturnSuccess) {
        IOLog("%s::%s::Could not add interrupt source to workloop\n", getName(), _dev->name);
        return -1;
    }
    
    _rmidev->interruptSource->enable();
    
    _rmidev->commandGate = IOCommandGate::commandGate(this);
    
    if (!_rmidev->commandGate || (_dev->workLoop->addEventSource(_rmidev->commandGate) != kIOReturnSuccess)) {
        IOLog("%s::%s::Failed to open RMI4 command gate\n", getName(), _dev->name);
        return -1;
    }

    
    return 0;
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
        
    _dev->workLoop->removeEventSource(_rmidev->interruptSource);
    _rmidev->commandGate->release();
    _rmidev->commandGate = NULL;
    
    _dev->workLoop->removeEventSource(_rmidev->interruptSource);
    _rmidev->interruptSource->disable();
    _rmidev->interruptSource = NULL;
    
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

    IOFree(_rmidev, sizeof(RMI4Device));
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
    UInt32 ic_con, ic_tar;
    
    writel(_dev, 0, DW_IC_ENABLE);
    
    ic_con = readl(_dev, DW_IC_CON);
    if(msgs[_dev->msg_write_idx].flags & I2C_M_TEN) {
        ic_con |= DW_IC_CON_10BITADDR_MASTER;
        ic_tar = DW_IC_TAR_10BITADDR_MASTER;
    } else
        ic_con &= ~DW_IC_CON_10BITADDR_MASTER;
    
    writel(_dev, ic_con, DW_IC_CON);
    
    writel(_dev, msgs[_dev->msg_write_idx].addr | ic_tar, DW_IC_TAR);
    
    disableI2CInt(_dev);
    
    writel(_dev, 1, DW_IC_ENABLE);
    
    clearI2CInt(_dev);
    
    writel(_dev, DW_IC_INTR_DEFAULT_MASK, DW_IC_INTR_MASK);
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
            IOLog("%s::%s::Invalid target address, aborting transaction\n", getName(), _dev->name);
            _dev->msg_err = -1;
            break;
        }
        
        if (msgs[_dev->msg_write_idx].len == 0) {
            IOLog("%s::%s::Invalid message length, aborting transaction\n", getName(), _dev->name);
            _dev->msg_err = -1;
            break;
        }
        
        if (!(_dev->status & STATUS_WRITE_IN_PROGRESS)) {
            buf = msgs[_dev->msg_write_idx].buf;
            buf_len = msgs[_dev->msg_write_idx].len;
            
            if ((_dev->master_cfg & DW_IC_CON_RESTART_EN) && (_dev->msg_write_idx > 0))
                need_restart = true;
        }
        
        tx_limit = _dev->tx_fifo_depth - readl(_dev, DW_IC_TXFLR);
        rx_limit = _dev->rx_fifo_depth - readl(_dev, DW_IC_RXFLR);
        
        while (buf_len > 0 && tx_limit > 0 && rx_limit > 0) {
            UInt32 cmd = 0;
            
            if (_dev->msg_write_idx == _dev->msgs_num - 1 && buf_len == 1)
                cmd |= BIT(9);
            
            if (need_restart) {
                cmd |= BIT(10);
                need_restart = false;
            }
            
            if (msgs[_dev->msg_write_idx].flags & I2C_M_RD) {
                
                if (rx_limit - _dev->rx_outstanding <= 0)
                    break;
                
                writel(_dev, cmd | 0x100, DW_IC_DATA_CMD);
                rx_limit--;
                _dev->rx_outstanding++;
            } else
                writel(_dev, cmd | *buf++, DW_IC_DATA_CMD);
            tx_limit--; buf_len--;
        }
        
        _dev->tx_buf = buf;
        _dev->tx_buf_len = buf_len;
        
        if (buf_len > 0) {
            _dev->status |= STATUS_WRITE_IN_PROGRESS;
            break;
        } else
            _dev->status &= ~STATUS_WRITE_IN_PROGRESS;
        
    }
   
    if (_dev->msg_write_idx == _dev->msgs_num)
        intr_mask &= ~DW_IC_INTR_TX_EMPTY;
    
    if (_dev->msg_err)
        intr_mask = 0;
    
    writel(_dev, intr_mask, DW_IC_INTR_MASK);
    
}

SInt32 VoodooI2C::i2c_smbus_read_i2c_block_data(I2CBus* phys, UInt16 addr, UInt8 command, UInt8 length, UInt8 *values) {
    i2c_smbus_data data;
    int status;
    
    if (length > I2C_SMBUS_BLOCK_MAX)
        length = I2C_SMBUS_BLOCK_MAX;
    
    data.block[0] = length;
    
    
    status = i2c_smbus_xfer(phys, addr, I2C_SMBUS_READ, command, I2C_SMBUS_I2C_BLOCK_DATA, &data);
    
    if (status < 0)
        return status;
    
    memcpy(values, &data.block[1], data.block[0]);
    return data.block[0];
}

SInt32 VoodooI2C::i2c_smbus_write_i2c_block_data(I2CBus* phys, UInt16 addr, UInt8 command, UInt8 length, const UInt8 *values) {
    i2c_smbus_data data;
    
    if(length >I2C_SMBUS_BLOCK_MAX)
        length = I2C_SMBUS_BLOCK_MAX;
    data.block[0] = length;
    memcpy(data.block + 1, values, length);
    
    return i2c_smbus_xfer(phys, _rmidev->addr, I2C_SMBUS_WRITE, command, I2C_SMBUS_I2C_BLOCK_DATA, &data);
}

SInt32 VoodooI2C::i2c_smbus_write_byte_data(I2CBus *phys, UInt16 addr, UInt8 command, UInt8 value) {
    i2c_smbus_data data;
    data.byte = value;
    return i2c_smbus_xfer(phys, addr, I2C_SMBUS_WRITE, command, I2C_SMBUS_BYTE_DATA, &data);
    
    
}


SInt32 VoodooI2C::i2c_smbus_xfer(I2CBus *phys,
                                 UInt16 addr, char read_write, UInt8 command, int size,
                                 i2c_smbus_data *data) {
    
    if (phys->commandGate){
        
        commandGateTransaction transaction;
        
        transaction.phys = phys;
        transaction.addr = addr;
        transaction.read_write = read_write;
        transaction.command = command;
        transaction.size = size;
        transaction.data = data;
        
        
        SInt32 result = phys->commandGate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooI2C::i2c_smbus_xfer_gated), &transaction);
        
        return result;
        
    }
    
    return -1;
    

}

SInt32 VoodooI2C::i2c_smbus_xfer_gated(commandGateTransaction *transaction) {
    
    I2CBus* phys = transaction->phys;
    UInt16 addr = transaction->addr;
    char read_write = transaction->read_write;
    UInt8 command = transaction->command;
    int size = transaction->size;
    i2c_smbus_data *data = transaction->data;
     
    
    unsigned char msgbuf0[I2C_SMBUS_BLOCK_MAX+3];
    unsigned char msgbuf1[I2C_SMBUS_BLOCK_MAX+2];
    int num = read_write == I2C_SMBUS_READ ? 2 : 1;
    int i;
    UInt8 partial_pec = 0;
    int status;
    
    struct i2c_msg msg[2] = {
        {
            .addr = addr,
            .flags = 0,
            .len = 1,
            .buf = msgbuf0,
        }, {
            .addr = addr,
            .flags = I2C_M_RD,
            .len = 0,
            .buf = msgbuf1,
        }
    };
    
    msgbuf0[0] = command;
    switch(size) {
        case I2C_SMBUS_QUICK:
            msg[0].len = 0;
            msg[0].flags = (read_write == I2C_SMBUS_READ ? I2C_M_RD : 0);
            num = 1;
            break;
        case I2C_SMBUS_BYTE:
            if (read_write == I2C_SMBUS_READ) {
                msg[0].flags = I2C_M_RD;
                num = 1;
            }
            break;
        case I2C_SMBUS_BYTE_DATA:
            if (read_write == I2C_SMBUS_READ)
                msg[1].len = 1;
            else {
                msg[0].len = 2;
                msgbuf0[1] = data->byte;
            }
        case I2C_SMBUS_WORD_DATA:
            if (read_write == I2C_SMBUS_READ)
                msg[1].len = 2;
            else {
                msg[0].len = 3;
                msgbuf0[1] = data->word & 0xff;
                msgbuf0[2] = data->word >> 8;
            }
            break;
        case I2C_SMBUS_PROC_CALL:
            num = 2;
            read_write = I2C_SMBUS_READ;
            msg[0].len = 3;
            msg[1].len = 2;
            msgbuf0[1] = data->word & 0xff;
            msgbuf0[2] = data->word >> 8;
            break;
        case I2C_SMBUS_BLOCK_DATA:
            if (read_write == I2C_SMBUS_READ ) {
                msg[1].flags = I2C_M_RECV_LEN;
                msg[1].len = 1;
            } else {
                msg[0].len = data->block[0] + 2;
                if (msg[0].len > I2C_SMBUS_BLOCK_MAX + 2) {
                    IOLog("invalid block write size");
                    return -1;
                }
                for (i=1; i < msg[0].len; i++)
                    msgbuf0[i] = data->block[i-1];
            }
            break;
        case I2C_SMBUS_BLOCK_PROC_CALL:
            num = 2;
            read_write = I2C_SMBUS_READ;
            if (data->block[0] > I2C_SMBUS_BLOCK_MAX) {
                IOLog("invalid block write size");
                return -1;
            }
            msg[0].len = data->block[0] + 2;
            for (i=1; i < msg[0].len; i++)
                msgbuf0[i] = data->block[i-1];
            msg[1].flags = I2C_M_RECV_LEN;
            msg[1].len = 1;
            break;
        case I2C_SMBUS_I2C_BLOCK_DATA:
            if (read_write == I2C_SMBUS_READ) {
                msg[1].len = data->block[0];
            } else {
                msg[0].len = data->block[0] + 1;
                if (msg[0].len > I2C_SMBUS_BLOCK_MAX + 1) {
                    IOLog("invalid block write size");
                    return -1;
                }
                for (i = 1; i <= data->block[0]; i++)
                    msgbuf0[i] = data->block[i];
            }
            break;
        default:
            IOLog("unsupported transaction");
            return -1;
    }
    
    
    
    status = i2c_transfer(phys, msg, num);
    if (status < 0)
        return status;
    
    
    if (read_write == I2C_SMBUS_READ) {
        switch(size) {
            case I2C_SMBUS_BYTE:
                data->byte = msgbuf0[0];
                break;
            case I2C_SMBUS_BYTE_DATA:
                data->byte = msgbuf1[0];
                break;
            case I2C_SMBUS_WORD_DATA:
            case I2C_SMBUS_PROC_CALL:
                data->word = msgbuf1[0] | (msgbuf1[1] << 8);
                break;
            case I2C_SMBUS_I2C_BLOCK_DATA:
                for (i=0; i < data->block[0]; i++)
                    data->block[i+1] = msgbuf1[i];
                break;
            case I2C_SMBUS_BLOCK_DATA:
            case I2C_SMBUS_BLOCK_PROC_CALL:
                for (i=0; i< msgbuf1[0] + 1; i++)
                    data->block[i] = msgbuf1[i];
                break;
        }
    }
    
    return 0;
}

int VoodooI2C::i2c_transfer(I2CBus* phys, i2c_msg *msgs, int num) {
    int ret;
    
    for (ret = 0 ; ret < num; ret++ ) {
        IOLog("master_xfer[%d] %s, addr=0x%02x, len=%d%s\n", ret, (msgs[ret].flags & I2C_M_RD) ? "R" : "W", msgs[ret].addr, msgs[ret].len, (msgs[ret].flags & I2C_M_RECV_LEN) ? "+" : "");
    }
    
    ret = __i2c_transfer(phys, msgs, num);

    
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
    
    IOLog("enabled=0x%x stat=0x%x\n", enabled, stat);

    if (stat & DW_IC_INTR_TX_ABRT) {
        IOLog("%s::%s::Interrupt Aborting transaction\n", getName(), _dev->name);

        _dev->cmd_err |= DW_IC_ERR_TX_ABRT;
        _dev->status = STATUS_IDLE;
        
        writel(_dev, 0, DW_IC_INTR_MASK);
        goto tx_aborted;
    }
    
    if (stat & DW_IC_INTR_RX_FULL) {
        IOLog("%s::%s::interrupt reading transaction\n", getName(), _dev->name);
        readI2C(_dev);
    }
    
    if (stat & DW_IC_INTR_TX_EMPTY) {
        IOLog("%s::%s::interrupt xfer transaction\n", getName(), _dev->name);
        xferMsgI2C(_dev);
    }
    
tx_aborted:
    if ((stat & (DW_IC_INTR_TX_ABRT | DW_IC_INTR_STOP_DET )) || _dev->msg_err)
        _dev->commandGate->commandWakeup(&_dev->commandComplete);
}

int VoodooI2C::rmi_set_page(RMI4Device *phys, UInt page) {
    rmi_i2c_data *data = phys->data;
    int rc;
    
    rc = i2c_smbus_write_byte_data(phys->_dev, phys->addr, RMI_PAGE_SELECT_REGISTER, page);
    
    if (rc < 0) {
        IOLog("%s::%s::Set page failed: %d\n", getName(), _dev->name, rc);
        return rc;
    }
    data->page = page;
    return 0;
}

int VoodooI2C::rmi_i2c_write_block(RMI4Device *phys, UInt16 addr, UInt8 *buf, int len) {
    
    SInt32 result = phys->commandGate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooI2C::rmi_i2c_write_block_gated), phys, &addr, buf, &len);
    
    return result;
}

int VoodooI2C::rmi_i2c_write_block_gated(RMI4Device *phys, UInt16 *addr, UInt8 *buf, int *len) {
    rmi_i2c_data *data = phys->data;
    int rc;
    
    if(RMI_I2C_PAGE(*addr) != data->page) {
        rc = rmi_set_page(phys, RMI_I2C_PAGE(*addr));
        if (rc<0)
            return rc;
    }
    
    rc = i2c_smbus_write_i2c_block_data(phys->_dev, phys->addr, *addr & 0xff, sizeof(buf), buf);
    
    return rc;
}

int VoodooI2C::rmi_i2c_write(RMI4Device *phys, UInt16 addr, UInt8 data) {
    int rc = rmi_i2c_write_block(phys, addr, &data, 1);
    return (rc < 0) ? rc : 0;
}

void VoodooI2C::RMI4InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    //IOLog("%s::%s::RMI4 Interrupt occured\n", getName(), _dev->name);

}

int VoodooI2C::rmi_i2c_read_block(RMI4Device *phys, UInt16 addr, UInt8 *buf, int len) {
    
    SInt32 result = phys->commandGate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooI2C::rmi_i2c_read_block_gated), phys, &addr, buf, &len);
    
    return result;
}

int VoodooI2C::rmi_i2c_read_block_gated(RMI4Device *phys, UInt16 *addr, UInt8 *buf, int *len) {
    rmi_i2c_data *data = phys->data;
    int rc;
    
    if (RMI_I2C_PAGE(*addr) != data->page) {
        rc = rmi_set_page(phys, RMI_I2C_PAGE(*addr));
        if ( rc < 0)
            return rc;
    }
    
    
    rc = i2c_smbus_read_i2c_block_data(phys->_dev, phys->addr, *addr & 0xff, *len, buf);
    
    return rc;
}

int VoodooI2C::rmi_i2c_read(RMI4Device *phys, UInt16 addr, UInt8 *buf) {
    int rc = rmi_i2c_read_block(phys, addr, buf, 1);
    return ( rc < 0) ? rc : 0;
}

void VoodooI2C::clearI2CInt(I2CBus* _dev) {
    readl(_dev, DW_IC_CLR_INTR);
}








