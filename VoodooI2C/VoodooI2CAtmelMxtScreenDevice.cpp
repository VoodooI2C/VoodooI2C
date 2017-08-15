//
//  VoodooI2CAtmelMxtScreenDevice.cpp
//  VoodooI2C
//
//  Created by CoolStar on 12/13/15.
//  Copyright © 2015 CoolStar. All rights reserved.
//  ported from crostouchscreen 2.3 for Windows
//

#include "VoodooI2CAtmelMxtScreenDevice.h"
#include "VoodooI2C.h"
#include "VoodooAtmelTouchWrapper.h"

OSDefineMetaClassAndStructors(VoodooI2CAtmelMxtScreenDevice, VoodooI2CDevice);

#ifndef ABS32
#define ABS32
inline int32_t abs(int32_t num){
    if (num < 0){
        return num * -1;
    }
    return num;
}
#endif

#define REPORTID_MTOUCH         0x01
#define REPORTID_FEATURE        0x02

//
// Multitouch specific report information
//

#define MULTI_TIPSWITCH_BIT    1
#define MULTI_CONFIDENCE_BIT   2

#define MULTI_MIN_COORDINATE   0x0000
#define MULTI_MAX_COORDINATE   0x7FFF

#define MULTI_MAX_COUNT        10

#define MT_TOUCH_COLLECTION0                                                    \
0xa1, 0x02,                         /*     COLLECTION (Logical)         */ \
0x09, 0x42,                         /*       USAGE (Tip Switch)         */ \
0x15, 0x00,                         /*       LOGICAL_MINIMUM (0)        */ \
0x25, 0x01,                         /*       LOGICAL_MAXIMUM (1)        */ \
0x75, 0x01,                         /*       REPORT_SIZE (1)            */ \
0x95, 0x01,                         /*       REPORT_COUNT (1)           */ \
0x81, 0x02,                         /*       INPUT (Data,Var,Abs)       */ \
0x95, 0x07,                         /*       REPORT_COUNT (7)           */ \
0x81, 0x03,                         /*       INPUT (Cnst,Ary,Abs)       */ \
0x75, 0x08,                         /*       REPORT_SIZE (8)            */ \
0x09, 0x51,                         /*       USAGE (Contact Identifier) */ \
0x95, 0x01,                         /*       REPORT_COUNT (1)           */ \
0x81, 0x02,                         /*       INPUT (Data,Var,Abs)       */ \
0x05, 0x01,                         /*       USAGE_PAGE (Generic Desk.. */ \
0x75, 0x10,                         /*       REPORT_SIZE (16)           */ \
0x55, 0x00,                         /*       UNIT_EXPONENT (0)          */ \
0x65, 0x00,                         /*       UNIT (None)                */ \
0x35, 0x00,                         /*       PHYSICAL_MINIMUM (0)       */ \
0x46, 0x00, 0x00,                   /*       PHYSICAL_MAXIMUM (0)       */


//0x26, 0x56, 0x05,                   /*       LOGICAL_MAXIMUM (1366)    */

#define MT_TOUCH_COLLECTION1												\
0x09, 0x30,                         /*       USAGE (X)                  */ \
0x81, 0x02,                         /*       INPUT (Data,Var,Abs)       */

//0x26, 0x00, 0x03,                   /*       LOGICAL_MAXIMUM (768)    */

#define MT_TOUCH_COLLECTION2												\
0x09, 0x31,                         /*       USAGE (Y)                  */ \
0x81, 0x02,                         /*       INPUT (Data,Var,Abs)       */ \
0x05, 0x0d,                         /*       USAGE PAGE (Digitizers)    */ \
0x09, 0x48,                         /*       USAGE (Width)              */ \
0x81, 0x02,                         /*       INPUT (Data,Var,Abs)       */ \
0x09, 0x49,                         /*       USAGE (Height)             */ \
0x81, 0x02,                         /*       INPUT (Data,Var,Abs)       */ \
0xc0,                               /*    END_COLLECTION                */

#if 0
0x26, 0x56, 0x05,                   /*       LOGICAL_MAXIMUM (1366)    */
0x26, 0x00, 0x03,                   /*       LOGICAL_MAXIMUM (768)    */
#endif

#define MT_REF_TOUCH_COLLECTION												\
MT_TOUCH_COLLECTION0 \
0x26, 0x00, 0x00,                   /*       LOGICAL_MAXIMUM (1366)    */ \
MT_TOUCH_COLLECTION1 \
0x26, 0x00, 0x00,                   /*       LOGICAL_MAXIMUM (768)    */ \
MT_TOUCH_COLLECTION2 \

#define USAGE_PAGE \
0x05, 0x0d,                         /*    USAGE_PAGE (Digitizers) */  \
0x09, 0x54,                         /*    USAGE (Contact Count) */  \
0x95, 0x01,                         /*    REPORT_COUNT (1) */  \
0x75, 0x08,                         /*    REPORT_SIZE (8) */  \
0x15, 0x00,                         /*    LOGICAL_MINIMUM (0) */  \
0x25, 0x08,                         /*    LOGICAL_MAXIMUM (8) */  \
0x81, 0x02,                         /*    INPUT (Data,Var,Abs) */  \
0x09, 0x55,                         /*    USAGE(Contact Count Maximum) */  \
0xb1, 0x02,                         /*    FEATURE (Data,Var,Abs) */  \

unsigned char atmeldesc[] = {
    //
    // Multitouch report starts here
    //
    0x05, 0x0d,                         // USAGE_PAGE (Digitizers)
    0x09, 0x04,                         // USAGE (Touch Screen)
    0xa1, 0x01,                         // COLLECTION (Application)
    0x85, REPORTID_MTOUCH,              //   REPORT_ID (Touch)
    0x09, 0x22,                         //   USAGE (Finger)
    MT_REF_TOUCH_COLLECTION
    MT_REF_TOUCH_COLLECTION
    MT_REF_TOUCH_COLLECTION
    MT_REF_TOUCH_COLLECTION
    MT_REF_TOUCH_COLLECTION
    MT_REF_TOUCH_COLLECTION
    MT_REF_TOUCH_COLLECTION
    MT_REF_TOUCH_COLLECTION
    MT_REF_TOUCH_COLLECTION
    MT_REF_TOUCH_COLLECTION
    USAGE_PAGE
    0xc0,                               // END_COLLECTION
};

typedef struct __attribute__((__packed__)) TOUCH
{
    
    UInt8      Status;
    
    UInt8      ContactID;
    
    UInt16    XValue;
    
    UInt16    YValue;
    
    UInt16    Width;
    
    UInt16    Height;
    
} PTOUCH;

typedef struct __attribute__((__packed__)) _ATMEL_MULTITOUCH_REPORT
{
    
    UInt8      ReportID;
    
    TOUCH     Touch[10];
    
    UInt8      ActualCount;
    
} AtmelMultitouchReport;

typedef struct __attribute__((__packed__)) _ATMEL_MAXCOUNT_REPORT
{
    
    UInt8         ReportID;
    
    UInt8         MaximumCount;
    
} AtmelMaxcountReport;


bool VoodooI2CAtmelMxtScreenDevice::attach(IOService * provider, IOService* child)
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

bool VoodooI2CAtmelMxtScreenDevice::probe(IOService* device) {
    
    
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

void VoodooI2CAtmelMxtScreenDevice::stop(IOService* device) {
    
    IOLog("I2C HID Device is stopping\n");
    
    destroy_wrapper();
    
    if (hid_device->interruptSource){
        hid_device->interruptSource->disable();
        hid_device->workLoop->removeEventSource(hid_device->interruptSource);
        hid_device->interruptSource->release();
        hid_device->interruptSource = NULL;
    }
    
    hid_device->workLoop->release();
    hid_device->workLoop = NULL;
    
    if (totsize > 0)
        IOFree(core.buf, totsize);
    
    IOFree(hid_device, sizeof(I2CDevice));
    
    PMstop();
    
    //hid_device->provider->close(this);
    
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void VoodooI2CAtmelMxtScreenDevice::detach( IOService * provider )
{
    assert(_controller == provider);
    _controller->release();
    _controller = 0;
    
    super::detach(provider);
}

size_t VoodooI2CAtmelMxtScreenDevice::mxt_obj_size(const mxt_object *obj)
{
    return obj->size_minus_one + 1;
}

size_t VoodooI2CAtmelMxtScreenDevice::mxt_obj_instances(const mxt_object *obj)
{
    return obj->instances_minus_one + 1;
}

mxt_object *VoodooI2CAtmelMxtScreenDevice::mxt_findobject(struct mxt_rollup *core, int type)
{
    int i;
    
    for (i = 0; i < core->nobjs; ++i) {
        if (core->objs[i].type == type)
            return(&core->objs[i]);
    }
    return NULL;
}

int
VoodooI2CAtmelMxtScreenDevice::mxt_read_reg(uint16_t reg, void *rbuf, int bytes)
{
    uint8_t wreg[2];
    wreg[0] = reg & 255;
    wreg[1] = reg >> 8;
    
    uint16_t nreg = ((uint16_t *)wreg)[0];
    
    int error = readI2C16(nreg, bytes, (uint8_t *)rbuf);
    return error;
}

int
VoodooI2CAtmelMxtScreenDevice::mxt_write_reg_buf(uint16_t reg, void *xbuf, int bytes)
{
    uint8_t wreg[2];
    wreg[0] = reg & 255;
    wreg[1] = reg >> 8;
    
    uint16_t nreg = ((uint16_t *)wreg)[0];
    return writeI2C16(nreg, bytes, (uint8_t *)xbuf);
}

int
VoodooI2CAtmelMxtScreenDevice::mxt_write_reg(uint16_t reg, uint8_t val)
{
    return mxt_write_reg_buf(reg, &val, 1);
}

int
VoodooI2CAtmelMxtScreenDevice::mxt_write_object_off(mxt_object *obj,
                     int offset, uint8_t val)
{
    uint16_t reg = obj->start_address;
    
    reg += offset;
    return mxt_write_reg(reg, val);
}

void VoodooI2CAtmelMxtScreenDevice::atmel_reset_device()
{
    mxt_write_object_off(cmdprocobj, MXT_CMDPROC_RESET_OFF, 1);
    IOSleep(100);
}

int VoodooI2CAtmelMxtScreenDevice::mxt_set_t7_power_cfg(uint8_t sleep)
{
    t7_config *new_config;
    t7_config deepsleep;
    deepsleep.active = deepsleep.idle = 0;
    t7_config active;
    active.active = 20;
    active.idle = 100;
    
    if (sleep == MXT_POWER_CFG_DEEPSLEEP)
        new_config = &deepsleep;
    else
        new_config = &active;
    return mxt_write_reg_buf(T7_address,
                                 new_config, sizeof(t7_cfg));
}

int VoodooI2CAtmelMxtScreenDevice::mxt_read_t9_resolution()
{
    t9_range range;
    unsigned char orient;
    
    mxt_object *resolutionobject = mxt_findobject(&core, MXT_TOUCH_MULTI_T9);
    
    mxt_read_reg(resolutionobject->start_address + MXT_T9_RANGE, &range, sizeof(range));
    
    mxt_read_reg(resolutionobject->start_address + MXT_T9_ORIENT, &orient, 1);
    
    /* Handle default values */
    if (range.x == 0)
        range.x = 1023;
    
    if (range.y == 0)
        range.y = 1023;
    
    if (orient & MXT_T9_ORIENT_SWITCH) {
        max_x = range.y + 1;
        max_y = range.x + 1;
    }
    else {
        max_x = range.x + 1;
        max_y = range.y + 1;
    }
    IOLog("%s::%s:: Screen Size: X: %d Y: %d\n", getName(), _controller->_dev->name, max_x, max_y);
    return 0;
}

int VoodooI2CAtmelMxtScreenDevice::mxt_read_t100_config()
{
    uint16_t range_x, range_y;
    uint8_t cfg, tchaux;
    uint8_t aux;
    
    mxt_object *resolutionobject = mxt_findobject(&core, MXT_TOUCH_MULTITOUCHSCREEN_T100);
    
    /* read touchscreen dimensions */
    mxt_read_reg(resolutionobject->start_address + MXT_T100_XRANGE, &range_x, sizeof(range_x));
    
    mxt_read_reg(resolutionobject->start_address + MXT_T100_YRANGE, &range_y, sizeof(range_y));
    
    /* read orientation config */
    mxt_read_reg(resolutionobject->start_address + MXT_T100_CFG1, &cfg, 1);
    
    if (cfg & MXT_T100_CFG_SWITCHXY) {
        max_x = range_y + 1;
        max_y = range_x + 1;
    }
    else {
        max_x = range_x + 1;
        max_y = range_y + 1;
    }
    
    mxt_read_reg(resolutionobject->start_address + MXT_T100_TCHAUX, &tchaux, 1);
    
    aux = 6;
    
    if (tchaux & MXT_T100_TCHAUX_VECT)
        t100_aux_vect = aux++;
    
    if (tchaux & MXT_T100_TCHAUX_AMPL)
        t100_aux_ampl = aux++;
    
    if (tchaux & MXT_T100_TCHAUX_AREA)
        t100_aux_area = aux++;
    IOLog("%s::%s:: Screen Size T100: X: %d Y: %d\n", getName(), _controller->_dev->name, max_x, max_y);
    return 0;
}

int VoodooI2CAtmelMxtScreenDevice::initHIDDevice(I2CDevice *hid_device) {
    PMinit();
    
    int ret;
    
    int blksize;
    uint32_t crc;
    IOLog("%s::%s::Initializing Touch Screen.\n", getName(), _controller->_dev->name);
    
    mxt_read_reg(0, &core.info, sizeof(core.info));
    
    core.nobjs = core.info.num_objects;
    
    if (core.nobjs < 0 || core.nobjs > 1024) {
        IOLog("%s::%s::init_device nobjs (%d) out of bounds\n", getName(), _controller->_dev->name,core.nobjs);
    }
    
    blksize = sizeof(core.info) +
    core.nobjs * sizeof(mxt_object);
    totsize = blksize + sizeof(mxt_raw_crc);
    
    core.buf = (uint8_t *)IOMalloc(totsize);
    
    mxt_read_reg(0, core.buf, totsize);
    
    crc = obp_convert_crc((mxt_raw_crc *)((uint8_t *)core.buf + blksize));
    
    if (obp_crc24(core.buf, blksize) != crc) {
        IOLog("%s::%s::init_device: configuration space "
                   "crc mismatch %08x/%08x\n", getName(), _controller->_dev->name,
                   crc, obp_crc24(core.buf, blksize));
    }
    else {
        IOLog("%s::%s::init_device: CRC Matched!\n", getName(), _controller->_dev->name);
    }
    
    core.objs = (mxt_object *)((uint8_t *)core.buf +
                               sizeof(core.info));
    
    msgprocobj = mxt_findobject(&core, MXT_GEN_MESSAGEPROCESSOR);
    cmdprocobj = mxt_findobject(&core, MXT_GEN_COMMANDPROCESSOR);
    
    int reportid = 1;
    for (int i = 0; i < core.nobjs; i++) {
        mxt_object *obj = &core.objs[i];
        uint8_t min_id, max_id;
        
        if (obj->num_report_ids) {
            min_id = reportid;
            reportid += obj->num_report_ids *
            mxt_obj_instances(obj);
            max_id = reportid - 1;
        }
        else {
            min_id = 0;
            max_id = 0;
        }
        
        switch (obj->type) {
            case MXT_GEN_MESSAGE_T5:
                if (info.family == 0x80 &&
                    info.version < 0x20) {
                    /*
                     * On mXT224 firmware versions prior to V2.0
                     * read and discard unused CRC byte otherwise
                     * DMA reads are misaligned.
                     */
                    T5_msg_size = mxt_obj_size(obj);
                }
                else {
                    /* CRC not enabled, so skip last byte */
                    T5_msg_size = mxt_obj_size(obj) - 1;
                }
                T5_address = obj->start_address;
                break;
            case MXT_GEN_COMMAND_T6:
                T6_reportid = min_id;
                T6_address = obj->start_address;
                break;
            case MXT_GEN_POWER_T7:
                T7_address = obj->start_address;
                break;
            case MXT_TOUCH_MULTI_T9:
                multitouch = MXT_TOUCH_MULTI_T9;
                T9_reportid_min = min_id;
                T9_reportid_max = max_id;
                num_touchids = obj->num_report_ids
                * mxt_obj_instances(obj);
                break;
            case MXT_SPT_MESSAGECOUNT_T44:
                T44_address = obj->start_address;
                break;
            case MXT_SPT_GPIOPWM_T19:
                T19_reportid = min_id;
                break;
            case MXT_TOUCH_MULTITOUCHSCREEN_T100:
                multitouch = MXT_TOUCH_MULTITOUCHSCREEN_T100;
                T100_reportid_min = min_id;
                T100_reportid_max = max_id;
                
                /* first two report IDs reserved */
                num_touchids = obj->num_report_ids - 2;
                break;
        }
    }
    
    max_reportid = reportid;
    
    if (multitouch == MXT_TOUCH_MULTI_T9)
        mxt_read_t9_resolution();
    else if (multitouch == MXT_TOUCH_MULTITOUCHSCREEN_T100)
        mxt_read_t100_config();
    
    if (multitouch == MXT_TOUCH_MULTI_T9 || multitouch == MXT_TOUCH_MULTITOUCHSCREEN_T100) {
        uint16_t max16_x[] = { max_x };
        uint16_t max16_y[] = { max_y };
        
        uint8_t *max_x8bit = (uint8_t *)max16_x;
        uint8_t *max_y8bit = (uint8_t *)max16_y;
        
        max_x_hid[0] = max_x8bit[0];
        max_x_hid[1] = max_x8bit[1];
        
        max_y_hid[0] = max_y8bit[0];
        max_y_hid[1] = max_y8bit[1];
    }
    
    atmel_reset_device();
    
    if (multitouch == MXT_TOUCH_MULTITOUCHSCREEN_T100){
        mxt_set_t7_power_cfg(MXT_POWER_CFG_RUN);
    } else {
        mxt_object *obj = mxt_findobject(&core, MXT_TOUCH_MULTI_T9);
        mxt_write_object_off(obj, MXT_T9_CTRL, 0x83);
    }
    
    for (int i=0; i < 20; i++){
        Flags[i] = 0;
        XValue[i] = 0;
        YValue[i] = 0;
        AREA[i] = 0;
    }
    TouchCount = 0;
    
    
    hid_device->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!hid_device->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _controller->_dev->name);
        stop(this);
        return -1;
    }
    
    hid_device->workLoop->retain();
    
    hid_device->interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CAtmelMxtScreenDevice::InterruptOccured), hid_device->provider);
    
    if (!hid_device->interruptSource) {
        IOLog("%s::%s::Interrupt Error!\n", getName(), _controller->_dev->name);
        goto err;
    }
    
    hid_device->workLoop->addEventSource(hid_device->interruptSource);
    hid_device->interruptSource->enable();
    
    hid_device->touchScreenIsAwake = true;
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

void VoodooI2CAtmelMxtScreenDevice::initialize_wrapper(void) {
    destroy_wrapper();
    
    IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
    _wrapper = new VoodooAtmelTouchWrapper;
    if (_wrapper->init()) {
        IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
        _wrapper->attach(this);
        _wrapper->start(this);
    }
    else {
        IOLog("VoodooI2C: %s, line %d\n", __FILE__, __LINE__);
        _wrapper->release();
        _wrapper = NULL;
    }
}

void VoodooI2CAtmelMxtScreenDevice::destroy_wrapper(void) {
    if (_wrapper != NULL) {
        _wrapper->terminate(kIOServiceRequired | kIOServiceSynchronous);
        _wrapper->release();
        _wrapper = NULL;
    }
}

#define EIO              5      /* I/O error */
#define ENOMEM          12      /* Out of memory */

SInt32 VoodooI2CAtmelMxtScreenDevice::readI2C16(uint16_t reg, size_t len, uint8_t *values){
    uint16_t buf[] = {
        reg
    };
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = sizeof(buf),
            .buf = (uint8_t *)buf,
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

SInt32 VoodooI2CAtmelMxtScreenDevice::writeI2C16(uint16_t reg, size_t len, uint8_t *values){
    uint16_t buf[] = {
        reg
    };
    uint8_t *intermbuf = (uint8_t *)IOMalloc(sizeof(buf) + len);
    memcpy(intermbuf, buf, sizeof(buf));
    memcpy(intermbuf + 2, values, len);
    struct VoodooI2C::i2c_msg msgs[] = {
        {
            .addr = hid_device->addr,
            .flags = 0,
            .len = (UInt16)(sizeof(buf) + len),
            .buf = intermbuf,
        }
    };
    int ret;
    
    ret = _controller->i2c_transfer((VoodooI2C::i2c_msg*)&msgs, ARRAY_SIZE(msgs));
    IOFree(intermbuf, sizeof(buf) + len);
    return ret;
}

IOReturn VoodooI2CAtmelMxtScreenDevice::setPowerState(unsigned long powerState, IOService *whatDevice){
    if (powerState == 0){
        hid_device->touchScreenIsAwake = false;
        
        IOLog("%s::Going to sleep!\n", getName());
    } else {
        if (!hid_device->touchScreenIsAwake){
            atmel_reset_device();
            
            hid_device->touchScreenIsAwake = true;
        } else {
            IOLog("%s::Touch screen already awake! Not reinitializing.\n", getName());
        }
    }
    return kIOPMAckImplied;
}

int VoodooI2CAtmelMxtScreenDevice::i2c_get_slave_address(I2CDevice* hid_device){
    OSObject* result = NULL;
    
    hid_device->provider->evaluateObject("_CRS", &result);
    
    OSData* data = OSDynamicCast(OSData, result);
    
    uint8_t *crs = (uint8_t *)data->getBytesNoCopy();
    VoodooI2CACPICRSParser crsParser;
    crsParser.parse_acpi_crs(crs, 0, data->getLength());
    
    if (crsParser.foundI2C)
        hid_device->addr = crsParser.i2cInfo.address;
    
    data->release();
    
    if (crsParser.foundI2C)
        return 0;
    else
        return -1;
}

int VoodooI2CAtmelMxtScreenDevice::ProcessMessagesUntilInvalid() {
    int count, read;
    uint8_t tries = 2;
    
    count = max_reportid;
    do {
        read = ReadAndProcessMessages(count);
        if (read < count)
            return 0;
    } while (--tries);
    return -1;
}

int VoodooI2CAtmelMxtScreenDevice::ProcessMessage(uint8_t *message) {
    uint8_t report_id = message[0];
    
    if (report_id == 0xff)
        return 0;
    
    if (report_id == T6_reportid) {
        uint8_t status = message[1];
        uint32_t crc = message[2] | (message[3] << 8) | (message[4] << 16);
    }
    else if (report_id >= T9_reportid_min && report_id <= T9_reportid_max) {
        uint8_t flags = message[1];
        
        int rawx = (message[2] << 4) | ((message[4] >> 4) & 0xf);
        int rawy = (message[3] << 4) | ((message[4] & 0xf));
        
        /* Handle 10/12 bit switching */
        if (max_x < 1024)
            rawx >>= 2;
        if (max_y < 1024)
            rawy >>= 2;
        
        uint8_t area = message[5];
        uint8_t ampl = message[6];
        
        Flags[report_id] = flags;
        XValue[report_id] = rawx;
        YValue[report_id] = rawy;
        AREA[report_id] = area;
    }
    else if (report_id >= T100_reportid_min && report_id <= T100_reportid_max) {
        int reportid = report_id - T100_reportid_min - 2;
        
        uint8_t flags = message[1];
        
        uint8_t t9_flags = 0; //convert T100 flags to T9
        if (flags & MXT_T100_DETECT)
            t9_flags += MXT_T9_DETECT;
        else if (Flags[reportid] & MXT_T100_DETECT)
            t9_flags += MXT_T9_RELEASE;
        
        int rawx = *((uint16_t *)&message[2]);
        int rawy = *((uint16_t *)&message[4]);
        
        if (reportid >= 0) {
            Flags[reportid] = t9_flags;
            
            XValue[reportid] = rawx;
            YValue[reportid] = rawy;
            AREA[reportid] = 10;
        }
    }
    
    struct _ATMEL_MULTITOUCH_REPORT report;
    report.ReportID = REPORTID_MTOUCH;
    
    for (int i=0;i<10;i++){
        report.Touch[i].ContactID = 0;
        report.Touch[i].Height = 0;
        report.Touch[i].Width = 0;
        report.Touch[i].XValue = 0;
        report.Touch[i].YValue = 0;
        report.Touch[i].Status = 0;
    }
    
    int count = 0, i = 0;
    while (count < 10 && i < 20) {
        if (Flags[i] != 0) {
            report.Touch[count].ContactID = i;
            report.Touch[count].Height = AREA[i];
            report.Touch[count].Width = AREA[i];
            
            report.Touch[count].XValue = XValue[i];
            report.Touch[count].YValue = YValue[i];
            
            uint8_t flags = Flags[i];
            if (flags & MXT_T9_DETECT) {
                report.Touch[count].Status = MULTI_TIPSWITCH_BIT;
            }
            else if (flags & MXT_T9_PRESS) {
                report.Touch[count].Status = MULTI_TIPSWITCH_BIT;
            }
            else if (flags & MXT_T9_RELEASE) {
                report.Touch[count].Status = 0;
                
                Flags[i] = 0;
                XValue[i] = 0;
                YValue[i] = 0;
                AREA[i] = 0;
            }
            else
                report.Touch[count].Status = 0;
            
            count++;
        }
        i++;
    }
    
    report.ActualCount = count;
    if (count > 0 || TouchCount != count) {
        IOLog("%s::%s::Touches %d\n", getName(), _controller->_dev->name, count);
        
        IOBufferMemoryDescriptor *buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, sizeof(report));
        buffer->writeBytes(0, &report, sizeof(report));
        
        IOReturn err = _wrapper->handleReport(buffer, kIOHIDReportTypeInput);
        if (err != kIOReturnSuccess)
            IOLog("Error handling report: 0x%.8x\n", err);
        
        buffer->release();
    }
    
    TouchCount = count;
    return 1;
}

int VoodooI2CAtmelMxtScreenDevice::ReadAndProcessMessages(uint8_t count) {
    uint8_t num_valid = 0;
    int i, ret;
    if (count > max_reportid)
        return -1;
    
    int msg_buf_size = max_reportid * T5_msg_size;
    uint8_t *msg_buf = (uint8_t *)IOMalloc(msg_buf_size);
    
    for (int i = 0; i < max_reportid * T5_msg_size; i++) {
        msg_buf[i] = 0xff;
    }
    
    mxt_read_reg(T5_address, msg_buf, T5_msg_size * count);
    
    for (i = 0; i < count; i++) {
        ret = ProcessMessage(msg_buf + T5_msg_size * i);
        
        if (ret == 1)
            num_valid++;
    }
    
    IOFree(msg_buf, msg_buf_size);
    
    /* return number of messages read */
    return num_valid;
}

bool VoodooI2CAtmelMxtScreenDevice::DeviceReadT44() {
    int stret, ret;
    uint8_t count, num_left;
    
    int msg_buf_size = T5_msg_size + 1;
    uint8_t *msg_buf = (uint8_t *)IOMalloc(msg_buf_size);
    
    /* Read T44 and T5 together */
    stret = mxt_read_reg(T44_address, msg_buf, T5_msg_size);
    
    count = msg_buf[0];
    
    if (count == 0)
        goto end;
    
    if (count > max_reportid) {
        count = max_reportid;
    }
    
    ret = ProcessMessage(msg_buf + 1);
    if (ret < 0) {
        goto end;
    }
    
    num_left = count - 1;
    
    if (num_left) {
        ret = ReadAndProcessMessages(num_left);
        if (ret < 0)
            goto end;
        //else if (ret != num_left)
        ///	DbgPrint("T44: Unexpected invalid message!\n");
    }
    
end:
    IOFree(msg_buf, msg_buf_size);
    return true;
}

bool VoodooI2CAtmelMxtScreenDevice::DeviceRead() {
    int total_handled, num_handled;
    uint8_t count = last_message_count;
    
    if (count < 1 || count > max_reportid)
        count = 1;
    
    /* include final invalid message */
    total_handled = ReadAndProcessMessages(count + 1);
    if (total_handled < 0)
        return false;
    else if (total_handled <= count)
        goto update_count;
    
    /* keep reading two msgs until one is invalid or reportid limit */
    do {
        num_handled = ReadAndProcessMessages(2);
        if (num_handled < 0)
            return false;
        
        total_handled += num_handled;
        
        if (num_handled < 2)
            break;
    } while (total_handled < num_touchids);
    
update_count:
    last_message_count = total_handled;
    
    return true;
}

static void atmelMxt_readReport(VoodooI2CAtmelMxtScreenDevice *device){
    if (device->T44_address)
        device->DeviceReadT44();
    else
        device->DeviceRead();
    device->hid_device->reading = false;
}

void VoodooI2CAtmelMxtScreenDevice::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    if (hid_device->reading)
        return;
    
    hid_device->reading = true;
    
    thread_t newThread;
    kern_return_t kr = kernel_thread_start((thread_continue_t)atmelMxt_readReport, this, &newThread);
    if (kr != KERN_SUCCESS){
        hid_device->reading = false;
        IOLog("Thread error!\n");
    } else {
        thread_deallocate(newThread);
    }
}

int VoodooI2CAtmelMxtScreenDevice::reportDescriptorLength(){
    return sizeof(atmeldesc);
}

int VoodooI2CAtmelMxtScreenDevice::vendorID(){
    return 'lmtA';
}

int VoodooI2CAtmelMxtScreenDevice::productID(){
    return 'stxM';
}

void VoodooI2CAtmelMxtScreenDevice::write_report_to_buffer(IOMemoryDescriptor *buffer){
    
    struct _ATMEL_MULTITOUCH_REPORT report;
    report.ReportID = REPORTID_MTOUCH;
    
    for (int i=0;i<10;i++){
        report.Touch[i].ContactID = 0;
        report.Touch[i].Height = 0;
        report.Touch[i].Width = 0;
        report.Touch[i].XValue = 0;
        report.Touch[i].YValue = 0;
        report.Touch[i].Status = 0;
    }
    
    int count = 0, i = 0;
    while (count < 10 && i < 20) {
        if (Flags[i] != 0) {
            report.Touch[count].ContactID = i;
            report.Touch[count].Height = AREA[i];
            report.Touch[count].Width = AREA[i];
            
            report.Touch[count].XValue = XValue[i];
            report.Touch[count].YValue = YValue[i];
            
            uint8_t flags = Flags[i];
            if (flags & MXT_T9_DETECT) {
                report.Touch[count].Status = MULTI_TIPSWITCH_BIT;
            }
            else if (flags & MXT_T9_PRESS) {
                report.Touch[count].Status = MULTI_TIPSWITCH_BIT;
            }
            else if (flags & MXT_T9_RELEASE) {
                report.Touch[count].Status = 0;
                
                report.Touch[count].XValue = 0;
                report.Touch[count].YValue = 0;
                report.Touch[count].Width = 0;
                report.Touch[count].Height = 0;
                
                Flags[i] = 0;
                XValue[i] = 0;
                YValue[i] = 0;
                AREA[i] = 0;
            }
            
            count++;
        }
        i++;
    }
    
    report.ActualCount = count;
    
    UInt rsize = sizeof(report);
    
    buffer->writeBytes(0, &report, rsize);
}

void VoodooI2CAtmelMxtScreenDevice::write_report_descriptor_to_buffer(IOMemoryDescriptor *buffer){
    
#define MT_TOUCH_COLLECTION												\
MT_TOUCH_COLLECTION0 \
0x26, max_x_hid[0], max_x_hid[1],                   /*       LOGICAL_MAXIMUM (WIDTH)    */ \
MT_TOUCH_COLLECTION1 \
0x26, max_y_hid[0], max_y_hid[1],                   /*       LOGICAL_MAXIMUM (HEIGHT)    */ \
MT_TOUCH_COLLECTION2 \

    unsigned char ReportDescriptor[] = {
        //
        // Multitouch report starts here
        //
        0x05, 0x0d,                         // USAGE_PAGE (Digitizers)
        0x09, 0x04,                         // USAGE (Touch Screen)
        0xa1, 0x01,                         // COLLECTION (Application)
        0x85, REPORTID_MTOUCH,              //   REPORT_ID (Touch)
        0x09, 0x22,                         //   USAGE (Finger)
        MT_TOUCH_COLLECTION
        MT_TOUCH_COLLECTION
        MT_TOUCH_COLLECTION
        MT_TOUCH_COLLECTION
        MT_TOUCH_COLLECTION
        MT_TOUCH_COLLECTION
        MT_TOUCH_COLLECTION
        MT_TOUCH_COLLECTION
        MT_TOUCH_COLLECTION
        MT_TOUCH_COLLECTION
        USAGE_PAGE
        0xc0,                               // END_COLLECTION
    };
    
    IOLog("%s::%s:: Report Descriptor Written\n", getName(), _controller->_dev->name);
    
    UInt rsize = sizeof(ReportDescriptor);
    
    buffer->writeBytes(0, ReportDescriptor, rsize);
}
