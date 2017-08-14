//
//  VoodooI2CAtmelMxtScreenDevice.h
//  VoodooI2C
//
//  Created by CoolStar on 12/13/15.
//  Copyright © 2015 CoolStar. All rights reserved.
//  ported from crostouchscreen 2.3 for Windows
//

#ifndef VoodooI2C_VoodooAtmelMxtScreenDevice_h
#define VoodooI2C_VoodooAtmelMxtScreenDevice_h

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOTimerEventSource.h>
#include "VoodooI2CDevice.h"
#include "atmel_mxt.h"

#define __le16 UInt16
#define __le32 UInt32

#define HID_MAX_DESCRIPTOR_SIZE 4096

class VoodooI2C;
class VoodooAtmelTouchWrapper;
class IOBufferMemoryDescriptor;

class VoodooI2CAtmelMxtScreenDevice : public VoodooI2CDevice
{
    typedef IOService super;
    OSDeclareDefaultStructors(VoodooI2CAtmelMxtScreenDevice);
    
private:
    VoodooAtmelTouchWrapper* _wrapper;
    
    void initialize_wrapper(void);
    void destroy_wrapper(void);
    
protected:
    VoodooI2C* _controller;
    
public:
    virtual bool attach(IOService * provider, IOService* child) override;
    virtual void detach(IOService * provider) override;
    void stop(IOService* device) override;
    
    
    typedef struct {
        
        unsigned short addr;
        
        void* _dev;
        
        IOWorkLoop* workLoop;
        
        IOInterruptEventSource *interruptSource;
        
        IOACPIPlatformDevice* provider;
        
        char* name;
        
        bool reading;
        
        bool touchScreenIsAwake;
        
    } I2CDevice;
    
    struct i2c_msg {
        UInt16 addr;
        UInt16 flags;
        UInt16 len;
        UInt8 *buf;
        
#define I2C_M_TEN 0x0010
#define I2C_M_RD 0x0001
#define I2C_M_RECV_LEN 0x0400
        
#define I2C_HID_READ_PENDING (1 << 2);
        
#define I2C_HID_CMD(opcode_) \
.opcode = opcode_, .length = 4,\
.registerIndex = offsetof(struct i2c_hid_desc, wCommandRegister)
    };
    
    struct {
        UInt8 x;
        UInt8 y;
        UInt8 buttonMask;
    } lastmouse;
    
    I2CDevice* hid_device;
    
    int initHIDDevice(I2CDevice *hid_device);
    
    bool probe(IOService* device);
    
    void InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    
    void get_input(OSObject* owner, IOTimerEventSource* sender);
    
    int reportDescriptorLength();
    
    int vendorID();
    int productID();
    
    void write_report_to_buffer(IOMemoryDescriptor *buffer);
    void write_report_descriptor_to_buffer(IOMemoryDescriptor *buffer);
    
    virtual IOReturn setPowerState(unsigned long powerState, IOService *whatDevice) override;
    
    int i2c_get_slave_address(I2CDevice* hid_device);
    
    mxt_rollup core;
    int totsize = 0;
    
    mxt_object	*msgprocobj;
    mxt_object	*cmdprocobj;
    
    mxt_id_info info;
    
    uint16_t max_x;
    uint16_t max_y;
    
    uint8_t max_x_hid[2];
    uint8_t max_y_hid[2];
    
    uint8_t num_touchids;
    uint8_t multitouch;
    
    t7_config t7_cfg;
    
    uint8_t t100_aux_ampl;
    uint8_t t100_aux_area;
    uint8_t t100_aux_vect;
    
    /* Cached parameters from object table */
    uint16_t T5_address;
    uint8_t T5_msg_size;
    uint8_t T6_reportid;
    uint16_t T6_address;
    uint16_t T7_address;
    uint8_t T9_reportid_min;
    uint8_t T9_reportid_max;
    uint8_t T19_reportid;
    uint16_t T44_address;
    uint8_t T100_reportid_min;
    uint8_t T100_reportid_max;
    
    uint8_t max_reportid;
    
    uint8_t last_message_count;
    
    uint32_t TouchCount;
    
    uint8_t      Flags[20];
    
    uint16_t    XValue[20];
    
    uint16_t    YValue[20];
    
    uint16_t    AREA[20];
    
    size_t mxt_obj_size(const mxt_object *obj);
    
    size_t mxt_obj_instances(const mxt_object *obj);
    
    mxt_object *mxt_findobject(struct mxt_rollup *core, int type);
    
    int mxt_read_reg(uint16_t reg, void *rbuf, int bytes);
    int mxt_write_reg_buf(uint16_t reg, void *xbuf, int bytes);
    int mxt_write_reg(uint16_t reg, uint8_t val);
    int mxt_write_object_off(mxt_object *obj,int offset, uint8_t val);
    void atmel_reset_device();
    int mxt_set_t7_power_cfg(uint8_t sleep);
    int mxt_read_t9_resolution();
    int mxt_read_t100_config();
    
    int ProcessMessagesUntilInvalid();
    int ProcessMessage(uint8_t *message);
    int ReadAndProcessMessages(uint8_t count);
    bool DeviceReadT44();
    bool DeviceRead();
    
    SInt32 readI2C16(uint16_t reg, size_t len, uint8_t *values);
    SInt32 writeI2C16(uint16_t reg, size_t len, uint8_t *values);
};


#endif
