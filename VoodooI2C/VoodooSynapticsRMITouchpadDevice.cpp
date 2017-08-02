//
//  VoodooSynapticsRMITouchpadDevice.cpp
//  VoodooI2C
//
//  Created by CoolStar on 12/19/16.
//  Copyright © 2016 Alexandre Daoud. All rights reserved.
//

#include "VoodooSynapticsRMITouchpadDevice.h"
#include "VoodooI2C.h"

#define BITS_PER_LONG 64
#define GENMASK(h, l) \
(((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))
#define BIT(nr)                 (1UL << (nr))
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

// Error codes
#define EPERM           1
#define ENOENT          2
#define ESRCH           3
#define EINTR           4
#define EIO             5
#define ENXIO           6
#define E2BIG           7
#define ENOEXEC         8
#define EBADF           9
#define ECHILD          10
#define EAGAIN          11
#define ENOMEM          12
#define EACCES          13
#define EFAULT          14
#define EBUSY           16
#define EEXIST          17
#define EXDEV           18
#define ENODEV          19
#define ENOTDIR         20
#define EISDIR          21
#define ENFILE          23
#define EMFILE          24
#define ENOTTY          25
#define EFBIG           27
#define ENOSPC          28
#define ESPIPE          29
#define EROFS           30
#define EMLINK          31
#define EPIPE           32
#define EDOM            33
#define EDEADLK         36
#define ENAMETOOLONG    38
#define ENOLCK          39
#define ENOSYS          40
#define ENOTEMPTY       41

OSDefineMetaClassAndStructors(VoodooSynapticsRMITouchpadDevice, VoodooI2CDevice);

void VoodooSynapticsRMITouchpadDevice::rmi_f11_process_touch(struct csgesture_softc *sc, int slot, uint8_t finger_state, uint8_t *touch_data)
{
    int x, y, wx, wy;
    int wide, major, minor;
    int z;
    
    if (finger_state == 0x01) {
        x = (touch_data[0] << 4) | (touch_data[2] & 0x0F);
        y = (touch_data[1] << 4) | (touch_data[2] >> 4);
        wx = touch_data[3] & 0x0F;
        wy = touch_data[3] >> 4;
        wide = (wx > wy);
        major = max(wx, wy);
        minor = min(wx, wy);
        z = touch_data[4];
        
        y = max_y - y;
        
        x *= sc->resx;
        x /= max_x;
        
        y *= sc->resy;
        y /= max_y;
        
        sc->x[slot] = x;
        sc->y[slot] = y;
        sc->p[slot] = z;
        /* y is inverted */
        //y = hdata->max_y - y;
        //printf("Touch %d: X: %d Y: %d Z: %d\n", slot, x, y, z);
    }
}

int VoodooSynapticsRMITouchpadDevice::rmi_f11_input(struct csgesture_softc *sc, uint8_t *rmiInput) {
    //begin rmi parse
    int offset;
    int i;
    
    offset = (max_fingers >> 2) + 1;
    for (i = 0; i < max_fingers; i++) {
        int fs_byte_position = i >> 2;
        int fs_bit_position = (i & 0x3) << 1;
        int finger_state = (rmiInput[fs_byte_position] >> fs_bit_position) &
        0x03;
        int position = offset + 5 * i;
        rmi_f11_process_touch(sc, i, finger_state, &rmiInput[position]);
    }
    return f11.report_size;
}

int VoodooSynapticsRMITouchpadDevice::rmi_f30_input(struct csgesture_softc *sc, uint8_t irq, uint8_t *rmiInput, int size)
{
    int i;
    bool value;
    
    if (!(irq & f30.irq_mask))
        return 0;
    
    if (size < (int)f30.report_size) {
        IOLog("%s::%s::Click Button pressed, but the click data is missing\n", getName(), _controller->_dev->name);
        return 0;
    }
    
    for (i = 0; i < gpio_led_count; i++) {
        if (button_mask & BIT(i)) {
            value = (rmiInput[i / 8] >> (i & 0x07)) & BIT(0);
            if (button_state_mask & BIT(i))
                value = !value;
            sc->buttondown = value;
        }
    }
    return f30.report_size;
}

void VoodooSynapticsRMITouchpadDevice::TrackpadRawInput(struct csgesture_softc *sc, uint8_t report[40], int tickinc){
    if (report[0] != RMI_ATTN_REPORT_ID)
        return;
    
    for (int i = 0;i < 5; i++) {
        sc->x[i] = -1;
        sc->y[i] = -1;
        sc->p[i] = -1;
    }
    
    int index = 2;
    
    int reportSize = 40;
    
    if (f11.interrupt_base < f30.interrupt_base) {
        index += rmi_f11_input(sc, &report[index]);
        index += rmi_f30_input(sc, report[1], &report[index], reportSize - index);
    }
    else {
        index += rmi_f30_input(sc, report[1], &report[index], reportSize - index);
        index += rmi_f11_input(sc, &report[index]);
    }
    
    _wrapper->ProcessGesture(sc);
}

bool VoodooSynapticsRMITouchpadDevice::attach(IOService * provider, IOService* child)
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

bool VoodooSynapticsRMITouchpadDevice::probe(IOService* device) {
    
    
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

void VoodooSynapticsRMITouchpadDevice::stop(IOService* device) {
    
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

void VoodooSynapticsRMITouchpadDevice::detach( IOService * provider )
{
    assert(_controller == provider);
    _controller->release();
    _controller = 0;
    
    super::detach(provider);
}

int VoodooSynapticsRMITouchpadDevice::initHIDDevice(I2CDevice *hid_device) {
    PMinit();
    
    int ret = 0;
    
    rmi_populate();
    
    csgesture_softc *sc = &softc;
    
    sprintf(sc->product_id, "unknown");
    sprintf(sc->firmware_version, "%ld", firmware_id);
    
    sc->resx = x_size_mm * 10;
    sc->resy = y_size_mm * 10;
    
    sc->phyx = max_x;
    sc->phyy = max_y;
    
    sc->frequency = 10;
    
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
    
    hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooSynapticsRMITouchpadDevice::get_input));
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
    
    hid_device->trackpadIsAwake = true;
    
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

void VoodooSynapticsRMITouchpadDevice::initialize_wrapper(void) {
    _wrapper = new CSGesture;
    _wrapper->vendorID = 'nalE';
    _wrapper->productID = 'dptE';
    _wrapper->softc = &softc;
    _wrapper->initialize_wrapper(this);
}

void VoodooSynapticsRMITouchpadDevice::destroy_wrapper(void) {
    _wrapper->destroy_wrapper();
}

#define EIO              5      /* I/O error */
#define ENOMEM          12      /* Out of memory */

int VoodooSynapticsRMITouchpadDevice::rmi_read_block(uint16_t addr, uint8_t *buf, const int len) {
    int ret = 0;
    
    if (RMI_PAGE(addr) != page) {
        ret = rmi_set_page(RMI_PAGE(addr));
        if (ret < 0)
            goto exit;
    }
    
    uint8_t writeReport[21];
    for (int i = 0; i < 21; i++) {
        writeReport[i] = 0;
    }
    writeReport[0] = RMI_READ_ADDR_REPORT_ID;
    writeReport[1] = 0;
    writeReport[2] = addr & 0xFF;
    writeReport[3] = (addr >> 8) & 0xFF;
    writeReport[4] = len & 0xFF;
    writeReport[5] = (len >> 8) & 0xFF;
    rmi_write_report(writeReport, sizeof(writeReport));
    
    uint8_t i2cInput[42];
    readI2C(sizeof(i2cInput), i2cInput);
    
    uint8_t rmiInput[40];
    for (int i = 0; i < 40; i++) {
        rmiInput[i] = i2cInput[i + 2];
    }
    if (rmiInput[0] == RMI_READ_DATA_REPORT_ID) {
        for (int i = 0; i < len; i++) {
            buf[i] = rmiInput[i + 2];
        }
    }
exit:
    return ret;
}

int VoodooSynapticsRMITouchpadDevice::rmi_write_report(uint8_t *report, size_t report_size){
    uint8_t command[25];
    command[0] = 0x25;
    command[1] = 0x00;
    command[2] = 0x17;
    command[3] = 0x00;
    for (int i = 0; i < report_size; i++) {
        command[i + 4] = report[i];
    }
    writeI2C(sizeof(command), command);
    return 0;
}

int VoodooSynapticsRMITouchpadDevice::rmi_read(uint16_t addr, uint8_t *buf){
    return rmi_read_block(addr, buf, 1);
}

int VoodooSynapticsRMITouchpadDevice::rmi_write_block(uint16_t addr, uint8_t *buf, const int len)
{
    int ret;
    
    uint8_t writeReport[21];
    for (int i = 0; i < 21; i++) {
        writeReport[i] = 0;
    }
    
    if (RMI_PAGE(addr) != page) {
        ret = rmi_set_page(RMI_PAGE(addr));
        if (ret < 0)
            goto exit;
    }
    
    writeReport[0] = RMI_WRITE_REPORT_ID;
    writeReport[1] = len;
    writeReport[2] = addr & 0xFF;
    writeReport[3] = (addr >> 8) & 0xFF;
    for (int i = 0; i < len; i++) {
        writeReport[i + 4] = buf[i];
    }
    
    ret = rmi_write_report(writeReport, sizeof(writeReport));
    if (ret < 0) {
        IOLog("%s::%s::failed to write request output report (%d)\n", getName(), _controller->_dev->name,
                  ret);
        goto exit;
    }
    ret = 0;
    
exit:
    return ret;
}

int VoodooSynapticsRMITouchpadDevice::rmi_set_page(uint8_t _page)
{
    uint8_t writeReport[21];
    int retval;
    
    writeReport[0] = RMI_WRITE_REPORT_ID;
    writeReport[1] = 1;
    writeReport[2] = 0xFF;
    writeReport[4] = _page;
    
    retval = rmi_write_report(writeReport,
                              sizeof(writeReport));
    
    page = _page;
    return retval;
}

int VoodooSynapticsRMITouchpadDevice::rmi_write(uint16_t addr, uint8_t *buf)
{
    return rmi_write_block(addr, buf, 1);
}

static unsigned long rmi_gen_mask(unsigned irq_base, unsigned irq_count)
{
    return GENMASK(irq_count + irq_base - 1, irq_base);
}

void VoodooSynapticsRMITouchpadDevice::rmi_register_function(struct pdt_entry *pdt_entry, int _page, unsigned interrupt_count)
{
    struct rmi_function *f = NULL;
    uint16_t page_base = page << 8;
    
    switch (pdt_entry->function_number) {
        case 0x01:
            f = &f01;
            break;
        case 0x11:
            f = &f11;
            break;
        case 0x30:
            f = &f30;
            break;
    }
    
    if (f) {
        f->page = _page;
        f->query_base_addr = page_base | pdt_entry->query_base_addr;
        f->command_base_addr = page_base | pdt_entry->command_base_addr;
        f->control_base_addr = page_base | pdt_entry->control_base_addr;
        f->data_base_addr = page_base | pdt_entry->data_base_addr;
        f->interrupt_base = interrupt_count;
        f->interrupt_count = pdt_entry->interrupt_source_count;
        f->irq_mask = rmi_gen_mask(f->interrupt_base,
                                   f->interrupt_count);
        interrupt_enable_mask |= f->irq_mask;
    }
}

int VoodooSynapticsRMITouchpadDevice::rmi_scan_pdt()
{
    struct pdt_entry entry;
    int _page;
    bool page_has_function;
    int i;
    int retval;
    int interrupt = 0;
    uint16_t page_start, pdt_start, pdt_end;
    
    IOLog("%s::%s::Scanning PDT...\n", getName(), _controller->_dev->name);
    
    for (_page = 0; (_page <= RMI4_MAX_PAGE); _page++) {
        page_start = RMI4_PAGE_SIZE * _page;
        pdt_start = page_start + PDT_START_SCAN_LOCATION;
        pdt_end = page_start + PDT_END_SCAN_LOCATION;
        
        page_has_function = false;
        for (i = pdt_start; i >= pdt_end; i -= sizeof(entry)) {
            retval = rmi_read_block(i, (uint8_t *)&entry, sizeof(entry));
            if (retval) {
                IOLog("%s::%s::Read of PDT entry at %#06x failed.\n", getName(), _controller->_dev->name,
                          i);
                goto error_exit;
            }
            
            if (RMI4_END_OF_PDT(entry.function_number))
                break;
            
            page_has_function = true;
            
            rmi_register_function(&entry, _page, interrupt);
            interrupt += entry.interrupt_source_count;
        }
        
        if (!page_has_function)
            break;
    }
    
    IOLog("%s::%s::Done with PDT scan.\n", getName(), _controller->_dev->name);
    retval = 0;
    
error_exit:
    return retval;
}

int VoodooSynapticsRMITouchpadDevice::rmi_populate_f01()
{
    uint8_t basic_queries[RMI_DEVICE_F01_BASIC_QUERY_LEN];
    uint8_t info[3];
    int ret;
    bool has_query42;
    bool has_lts;
    bool has_sensor_id;
    bool has_ds4_queries = false;
    bool has_build_id_query = false;
    bool has_package_id_query = false;
    uint16_t query_offset = f01.query_base_addr;
    uint16_t prod_info_addr;
    uint8_t ds4_query_len;
    
    ret = rmi_read_block(query_offset, basic_queries,
                         RMI_DEVICE_F01_BASIC_QUERY_LEN);
    if (ret) {
        IOLog("%s::%s::Can not read basic queries from Function 0x1.\n", getName(), _controller->_dev->name);
        return ret;
    }
    
    has_lts = !!(basic_queries[0] & BIT(2));
    has_sensor_id = !!(basic_queries[1] & BIT(3));
    has_query42 = !!(basic_queries[1] & BIT(7));
    
    query_offset += 11;
    prod_info_addr = query_offset + 6;
    query_offset += 10;
    
    if (has_lts)
        query_offset += 20;
    
    if (has_sensor_id)
        query_offset++;
    
    if (has_query42) {
        ret = rmi_read(query_offset, info);
        if (ret) {
            IOLog("%s::%s::Can not read query42.\n", getName(), _controller->_dev->name);
            return ret;
        }
        has_ds4_queries = !!(info[0] & BIT(0));
        query_offset++;
    }
    
    if (has_ds4_queries) {
        ret = rmi_read(query_offset, &ds4_query_len);
        if (ret) {
            IOLog("%s::%s::Can not read DS4 Query length.\n", getName(), _controller->_dev->name);
            return ret;
        }
        query_offset++;
        
        if (ds4_query_len > 0) {
            ret = rmi_read(query_offset, info);
            if (ret) {
                IOLog("%s::%s::Can not read DS4 query.\n", getName(), _controller->_dev->name);
                return ret;
            }
            
            has_package_id_query = !!(info[0] & BIT(0));
            has_build_id_query = !!(info[0] & BIT(1));
        }
    }
    
    if (has_package_id_query)
        prod_info_addr++;
    
    if (has_build_id_query) {
        ret = rmi_read_block(prod_info_addr, info, 3);
        if (ret) {
            IOLog("%s::%s::Can not read product info.\n", getName(), _controller->_dev->name);
            return ret;
        }
        
        firmware_id = info[1] << 8 | info[0];
        firmware_id += info[2] * 65536;
    }
    
    ret = rmi_read_block(f01.control_base_addr, info,
                         2);
    
    if (ret) {
        IOLog("%s::%s::can not read f01 ctrl registers\n", getName(), _controller->_dev->name);
        return ret;
    }
    
    f01_ctrl0 = info[0];
    
    if (!info[1]) {
        /*
         * Do to a firmware bug in some touchpads the F01 interrupt
         * enable control register will be cleared on reset.
         * This will stop the touchpad from reporting data, so
         * if F01 CTRL1 is 0 then we need to explicitly enable
         * interrupts for the functions we want data for.
         */
        restore_interrupt_mask = true;
        
        ret = rmi_write(f01.control_base_addr + 1,
                        &interrupt_enable_mask);
        if (ret) {
            IOLog("%s::%s::can not write to control reg 1: %d.\n", getName(), _controller->_dev->name, ret);
            return ret;
        }
        IOLog("%s::%s::Firmware bug fix needed!!! :/\n", getName(), _controller->_dev->name);
    }
    
    return 0;
}

int VoodooSynapticsRMITouchpadDevice::rmi_populate_f11()
{
    uint8_t buf[20];
    int ret;
    bool has_query9;
    bool has_query10 = false;
    bool has_query11;
    bool has_query12;
    bool has_query27;
    bool has_query28;
    bool has_query36 = false;
    bool has_physical_props;
    bool has_gestures;
    bool has_rel;
    bool has_data40 = false;
    bool has_dribble = false;
    bool has_palm_detect = false;
    unsigned x_size, y_size;
    uint16_t query_offset;
    
    if (!f11.query_base_addr) {
        IOLog("%s::%s::No 2D sensor found, giving up.\n", getName(), _controller->_dev->name);
        return -ENODEV;
    }
    
    /* query 0 contains some useful information */
    ret = rmi_read(f11.query_base_addr, buf);
    if (ret) {
        IOLog("%s::%s::can not get query 0: %d.\n", getName(), _controller->_dev->name, ret);
        return ret;
    }
    has_query9 = !!(buf[0] & BIT(3));
    has_query11 = !!(buf[0] & BIT(4));
    has_query12 = !!(buf[0] & BIT(5));
    has_query27 = !!(buf[0] & BIT(6));
    has_query28 = !!(buf[0] & BIT(7));
    
    /* query 1 to get the max number of fingers */
    ret = rmi_read(f11.query_base_addr + 1, buf);
    if (ret) {
        IOLog("%s::%s::can not get NumberOfFingers: %d.\n", getName(), _controller->_dev->name, ret);
        return ret;
    }
    max_fingers = (buf[0] & 0x07) + 1;
    if (max_fingers > 5)
        max_fingers = 10;
    
    f11.report_size = max_fingers * 5 +
    DIV_ROUND_UP(max_fingers, 4);
    
    if (!(buf[0] & BIT(4))) {
        IOLog("%s::%s::No absolute events, giving up.\n", getName(), _controller->_dev->name);
        return -ENODEV;
    }
    
    has_rel = !!(buf[0] & BIT(3));
    has_gestures = !!(buf[0] & BIT(5));
    
    ret = rmi_read(f11.query_base_addr + 5, buf);
    if (ret) {
        IOLog("%s::%s::can not get absolute data sources: %d.\n", getName(), _controller->_dev->name, ret);
        return ret;
    }
    
    has_dribble = !!(buf[0] & BIT(4));
    
    /*
     * At least 4 queries are guaranteed to be present in F11
     * +1 for query 5 which is present since absolute events are
     * reported and +1 for query 12.
     */
    query_offset = 6;
    
    if (has_rel)
        ++query_offset; /* query 6 is present */
    
    if (has_gestures) {
        /* query 8 to find out if query 10 exists */
        ret = rmi_read(f11.query_base_addr + query_offset + 1, buf);
        if (ret) {
            IOLog("%s::%s::can not read gesture information: %d.\n", getName(), _controller->_dev->name,
                      ret);
            return ret;
        }
        has_palm_detect = !!(buf[0] & BIT(0));
        has_query10 = !!(buf[0] & BIT(2));
        
        query_offset += 2; /* query 7 and 8 are present */
    }
    
    if (has_query9)
        ++query_offset;
    
    if (has_query10)
        ++query_offset;
    
    if (has_query11)
        ++query_offset;
    
    /* query 12 to know if the physical properties are reported */
    if (has_query12) {
        ret = rmi_read(f11.query_base_addr
                       + query_offset, buf);
        if (ret) {
            IOLog("%s::%s::can not get query 12: %d.\n", getName(), _controller->_dev->name, ret);
            return ret;
        }
        has_physical_props = !!(buf[0] & BIT(5));
        
        if (has_physical_props) {
            query_offset += 1;
            ret = rmi_read_block(f11.query_base_addr
                                 + query_offset, buf, 4);
            if (ret) {
                IOLog("%s::%s::can not read query 15-18: %d.\n", getName(), _controller->_dev->name,
                          ret);
                return ret;
            }
            
            x_size = buf[0] | (buf[1] << 8);
            y_size = buf[2] | (buf[3] << 8);
            
            x_size_mm = x_size / 10;
            y_size_mm = y_size / 10;
            
            IOLog("%s::%s::size in mm: %d x %d\n", getName(), _controller->_dev->name,
                      x_size_mm, y_size_mm);
            
            /*
             * query 15 - 18 contain the size of the sensor
             * and query 19 - 26 contain bezel dimensions
             */
            query_offset += 12;
        }
    }
    
    if (has_query27)
        ++query_offset;
    
    if (has_query28) {
        ret = rmi_read(f11.query_base_addr
                       + query_offset, buf);
        if (ret) {
            IOLog("%s::%s::can not get query 28: %d.\n", getName(), _controller->_dev->name, ret);
            return ret;
        }
        
        has_query36 = !!(buf[0] & BIT(6));
    }
    
    if (has_query36) {
        query_offset += 2;
        ret = rmi_read(f11.query_base_addr
                       + query_offset, buf);
        if (ret) {
            IOLog("%s::%s::can not get query 36: %d.\n", getName(), _controller->_dev->name, ret);
            return ret;
        }
        
        has_data40 = !!(buf[0] & BIT(5));
    }
    
    
    if (has_data40)
        f11.report_size += max_fingers * 2;
    
    ret = rmi_read_block(f11.control_base_addr,
                         f11_ctrl_regs, RMI_F11_CTRL_REG_COUNT);
    if (ret) {
        IOLog("%s::%s::can not read ctrl block of size 11: %d.\n", getName(), _controller->_dev->name, ret);
        return ret;
    }
    
    /* data->f11_ctrl_regs now contains valid register data */
    read_f11_ctrl_regs = true;
    
    max_x = f11_ctrl_regs[6] | (f11_ctrl_regs[7] << 8);
    max_y = f11_ctrl_regs[8] | (f11_ctrl_regs[9] << 8);
    
    IOLog("%s::%s::Trackpad Resolution: %d x %d\n", getName(), _controller->_dev->name, max_x, max_y);
    
    if (has_dribble) {
        f11_ctrl_regs[0] = f11_ctrl_regs[0] & ~BIT(6);
        ret = rmi_write(f11.control_base_addr,
                        f11_ctrl_regs);
        if (ret) {
            IOLog("%s::%s::can not write to control reg 0: %d.\n", getName(), _controller->_dev->name,
                      ret);
            return ret;
        }
    }
    
    if (has_palm_detect) {
        f11_ctrl_regs[11] = f11_ctrl_regs[11] & ~BIT(0);
        ret = rmi_write(f11.control_base_addr + 11,
                        &f11_ctrl_regs[11]);
        if (ret) {
            IOLog("%s::%s::can not write to control reg 11: %d.\n", getName(), _controller->_dev->name,
                      ret);
            return ret;
        }
    }
    
    return 0;
}

int VoodooSynapticsRMITouchpadDevice::rmi_populate_f30()
{
    uint8_t buf[20];
    int ret;
    bool has_gpio, has_led;
    unsigned bytes_per_ctrl;
    uint8_t ctrl2_addr;
    int ctrl2_3_length;
    int i;
    
    /* function F30 is for physical buttons */
    if (!f30.query_base_addr) {
        IOLog("%s::%s::No GPIO/LEDs found, giving up.\n", getName(), _controller->_dev->name);
        return -ENODEV;
    }
    
    ret = rmi_read_block(f30.query_base_addr, buf, 2);
    if (ret) {
        IOLog("%s::%s::can not get F30 query registers: %d.\n", getName(), _controller->_dev->name, ret);
        return ret;
    }
    
    has_gpio = !!(buf[0] & BIT(3));
    has_led = !!(buf[0] & BIT(2));
    gpio_led_count = buf[1] & 0x1f;
    
    /* retrieve ctrl 2 & 3 registers */
    bytes_per_ctrl = (gpio_led_count + 7) / 8;
    /* Ctrl0 is present only if both has_gpio and has_led are set*/
    ctrl2_addr = (has_gpio && has_led) ? bytes_per_ctrl : 0;
    /* Ctrl1 is always be present */
    ctrl2_addr += bytes_per_ctrl;
    ctrl2_3_length = 2 * bytes_per_ctrl;
    
    f30.report_size = bytes_per_ctrl;
    
    ret = rmi_read_block(f30.control_base_addr + ctrl2_addr,
                         buf, ctrl2_3_length);
    if (ret) {
        IOLog("%s::%s::can not read ctrl 2&3 block of size %d: %d.\n", getName(), _controller->_dev->name,
                  ctrl2_3_length, ret);
        return ret;
    }
    
    button_count = 0;
    button_mask = 0;
    button_state_mask = 0;
    
    for (i = 0; i < gpio_led_count; i++) {
        int byte_position = i >> 3;
        int bit_position = i & 0x07;
        uint8_t dir_byte = buf[byte_position];
        uint8_t data_byte = buf[byte_position + bytes_per_ctrl];
        bool dir = (dir_byte >> bit_position) & BIT(0);
        bool dat = (data_byte >> bit_position) & BIT(0);
        
        if (dir == 0) {
            /* input mode */
            if (dat) {
                /* actual buttons have pull up resistor */
                button_count++;
                button_mask += BIT(i);
                button_state_mask += BIT(i);
            }
        }
        
    }
    
    return 0;
}

int VoodooSynapticsRMITouchpadDevice::rmi_set_mode(uint8_t mode) {
    uint8_t command[] = { 0x22, 0x00, 0x3f, 0x03, 0x0f, 0x23, 0x00, 0x04, 0x00, RMI_SET_RMI_MODE_REPORT_ID, mode }; //magic bytes from Linux
    writeI2C(sizeof(command), command);
    return 0;
}

int VoodooSynapticsRMITouchpadDevice::rmi_populate() {
    int ret;
    
    ret = rmi_set_mode(RMI_MODE_ATTN_REPORTS);
    if (ret) {
        IOLog("%s::%s::PDT set mode failed with code %d\n", getName(), _controller->_dev->name, ret);
        return ret;
    }
    
    ret = rmi_scan_pdt();
    if (ret) {
        IOLog("%s::%s::PDT scan failed with code %d\n", getName(), _controller->_dev->name, ret);
        return ret;
    }
    
    ret = rmi_populate_f01();
    if (ret) {
        IOLog("%s::%s::Error while initializing F01 (%d)\n", getName(), _controller->_dev->name, ret);
        return ret;
    }
    
    ret = rmi_populate_f11();
    if (ret) {
        IOLog("%s::%s::Error while initializing F11 (%d)\n", getName(), _controller->_dev->name, ret);
        return ret;
    }
    
    ret = rmi_populate_f30();
    if (ret) {
        IOLog("%s::%s::Error while initializing F30 (%d)\n", getName(), _controller->_dev->name, ret);
        return ret;
    }
    
    return 0;
}

SInt32 VoodooSynapticsRMITouchpadDevice::readI2C(size_t len, uint8_t *values){
    struct VoodooI2C::i2c_msg msgs[] = {
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

SInt32 VoodooSynapticsRMITouchpadDevice::writeI2C(size_t len, uint8_t *values){
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

IOReturn VoodooSynapticsRMITouchpadDevice::setPowerState(unsigned long powerState, IOService *whatDevice){
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
            rmi_populate();
            
            hid_device->trackpadIsAwake = true;
            IOLog("%s::Woke up from Sleep!\n", getName());
        } else {
            IOLog("%s::Trackpad already awake! Not reinitializing.\n", getName());
        }
        
        //Waking up from Sleep
        if (!hid_device->timerSource){
            hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooSynapticsRMITouchpadDevice::get_input));
            hid_device->workLoop->addEventSource(hid_device->timerSource);
            hid_device->timerSource->setTimeoutMS(10);
        }
        
        if (_wrapper)
            _wrapper->wakeFromSleep();
    }
    return kIOPMAckImplied;
}

int VoodooSynapticsRMITouchpadDevice::i2c_get_slave_address(I2CDevice* hid_device){
    OSObject* result = NULL;
    
    hid_device->provider->evaluateObject("_CRS", &result);
    
    OSData* data = OSDynamicCast(OSData, result);
    
    hid_device->addr = *(int*)data->getBytesNoCopy(16,1) & 0xFF;
    
    data->release();
    
    return 0;
    
}

void VoodooSynapticsRMITouchpadDevice::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    IOLog("interrupt\n");
    if (hid_device->reading)
        return;
    //i2c_hid_get_input(ihid);
}

void VoodooSynapticsRMITouchpadDevice::get_input(OSObject* owner, IOTimerEventSource* sender) {
    uint8_t reg = 0;
    writeI2C(1, &reg);
    
    uint8_t i2cInput[42];
    readI2C(sizeof(i2cInput), i2cInput);
    
    uint8_t rmiInput[40];
    for (int i = 0; i < 40; i++) {
        rmiInput[i] = i2cInput[i + 2];
    }
    
    if (rmiInput[0] == 0x00){
        hid_device->timerSource->setTimeoutMS(10);
        return;
    }
    
    if (rmiInput[0] != RMI_ATTN_REPORT_ID) {
        hid_device->timerSource->setTimeoutMS(10);
        return;
    }
    
    IOLog("Got Input!\n");
    
    TrackpadRawInput(&softc, rmiInput, 1);
    hid_device->timerSource->setTimeoutMS(10);
}
