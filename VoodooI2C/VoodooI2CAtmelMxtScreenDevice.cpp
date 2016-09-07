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
#define MULTI_IN_RANGE_BIT     2
#define MULTI_CONFIDENCE_BIT   4

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
0x09, 0x32,                         /*       USAGE (In Range)           */ \
0x81, 0x02,                         /*       INPUT (Data,Var,Abs)       */ \
0x09, 0x47,                         /*       USAGE (Confidence)         */ \
0x81, 0x02,                         /*       INPUT (Data,Var,Abs)       */ \
0x95, 0x05,                         /*       REPORT_COUNT (5)           */ \
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
    
    assert(_controller == 0);
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
    
    if (hid_device->timerSource){
        hid_device->timerSource->cancelTimeout();
        hid_device->timerSource->release();
        hid_device->timerSource = NULL;
    }
    
    if (hid_device->interruptSource){
        //hid_device->interruptSource->disable();
        //hid_device->workLoop->removeEventSource(hid_device->interruptSource);
        hid_device->interruptSource = NULL;
    }
    
    hid_device->workLoop->release();
    hid_device->workLoop = NULL;
    
    if (totsize > 0)
        IOFree(core.buf, totsize);
    
    IOFree(hid_device, sizeof(I2CDevice));
    
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
        max_x = range.y+1;
        max_y = range.x+1;
    }
    else {
        max_x = range.x+1;
        max_y = range.y+1;
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
        IOLog("%s::%s::Obj Type: %d\n", getName(), _controller->_dev->name, obj->type);
    }
    
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
    
    for (int i=0; i < 20; i++){
        Flags[i] = 0;
    }
    TouchCount = 0;
    
    
    hid_device->workLoop = (IOWorkLoop*)getWorkLoop();
    if(!hid_device->workLoop) {
        IOLog("%s::%s::Failed to get workloop\n", getName(), _controller->_dev->name);
        stop(this);
        return -1;
    }
    
    hid_device->workLoop->retain();
    
    
     /*hid_device->interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventAction, this, &VoodooI2CAtmelMxtScreenDevice::InterruptOccured), hid_device->provider);
     
     if (hid_device->workLoop->addEventSource(hid_device->interruptSource) != kIOReturnSuccess) {
     IOLog("%s::%s::Could not add interrupt source to workloop\n", getName(), _controller->_dev->name);
     stop(this);
     return -1;
     }
     
     hid_device->interruptSource->enable();*/
    
    
    hid_device->timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &VoodooI2CAtmelMxtScreenDevice::get_input));
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

SInt32 VoodooI2CAtmelMxtScreenDevice::readI2C(uint8_t reg, size_t len, uint8_t *values){
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

SInt32 VoodooI2CAtmelMxtScreenDevice::writeI2C(uint8_t reg, size_t len, uint8_t *values){
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

SInt32 VoodooI2CAtmelMxtScreenDevice::writeI2C16(uint16_t reg, size_t len, uint8_t *values){
    uint16_t buf[] = {
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

int VoodooI2CAtmelMxtScreenDevice::i2c_get_slave_address(I2CDevice* hid_device){
    OSObject* result = NULL;
    
    hid_device->provider->evaluateObject("_CRS", &result);
    
    OSData* data = OSDynamicCast(OSData, result);
    
    hid_device->addr = *(int*)data->getBytesNoCopy(16,1) & 0xFF;
    
    data->release();
    
    return 0;
    
}

void VoodooI2CAtmelMxtScreenDevice::InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount){
    IOLog("interrupt\n");
    if (hid_device->reading)
        return;
    //i2c_hid_get_input(ihid);
}

void VoodooI2CAtmelMxtScreenDevice::get_input(OSObject* owner, IOTimerEventSource* sender) {
    //return;
    //mxt_message_t msg;
    //int ret1 = readI2C(0, sizeof(msg), (uint8_t *)&msg);
    mxt_message_t msg;
    int ret1 = mxt_read_reg(msgprocobj->start_address, &msg, sizeof(msg));
    
    int reportid = msg.any.reportid;
    
    if (reportid == 0xff) {
    }
    else if (reportid >= T9_reportid_min && reportid <= T9_reportid_max) {
        reportid = reportid - T9_reportid_min;
        
        int rawx = (msg.touch_t9.pos[0] << 4) |
        ((msg.touch_t9.pos[2] >> 4) & 0x0F);
        int rawy = ((msg.touch_t9.pos[1] << 4) |
                    ((msg.touch_t9.pos[2]) & 0x0F)) / 4;
        
        Flags[reportid] = msg.touch_t9.flags;
        XValue[reportid] = rawx;
        YValue[reportid] = rawy;
        AREA[reportid] = msg.touch_t9.area;
    }
    else if (reportid >= T100_reportid_min && reportid <= T100_reportid_max) {
        reportid = reportid - T100_reportid_min - 2;
        
        uint8_t t9_flags = 0; //convert T100 flags to T9
        if (msg.touch_t100.flags & MXT_T100_DETECT)
            t9_flags += MXT_T9_DETECT;
        else if (Flags[reportid] & MXT_T100_DETECT)
            t9_flags += MXT_T9_RELEASE;
        
        int rawx = msg.touch_t100.x;
        int rawy = msg.touch_t100.y;
        
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
                report.Touch[count].Status = MULTI_IN_RANGE_BIT | MULTI_CONFIDENCE_BIT | MULTI_TIPSWITCH_BIT;
                IOLog("%s::%s::Detect Touch %d\n", getName(), _controller->_dev->name, i);
            }
            else if (flags & MXT_T9_PRESS) {
                report.Touch[count].Status = MULTI_IN_RANGE_BIT | MULTI_CONFIDENCE_BIT | MULTI_TIPSWITCH_BIT;
                IOLog("%s::%s::Pressed Touch %d\n", getName(), _controller->_dev->name, i);
            }
            else if (flags & MXT_T9_RELEASE) {
                IOLog("%s::%s::Released Touch %d\n", getName(), _controller->_dev->name, i);
                report.Touch[count].Status = 0;
                Flags[i] = 0;
            }
            
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
    
    hid_device->timerSource->setTimeoutMS(10);
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
                report.Touch[count].Status = MULTI_IN_RANGE_BIT | MULTI_CONFIDENCE_BIT | MULTI_TIPSWITCH_BIT;
            }
            else if (flags & MXT_T9_PRESS) {
                report.Touch[count].Status = MULTI_IN_RANGE_BIT | MULTI_CONFIDENCE_BIT | MULTI_TIPSWITCH_BIT;
            }
            else if (flags & MXT_T9_RELEASE) {
                report.Touch[count].Status = 0;
                Flags[i] = 0;
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
