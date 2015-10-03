//
//  VoodooI2CDevice.h
//  VoodooI2C
//
//  Created by Alexandre on 02/02/2015.
//  Copyright (c) 2015 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2C_VoodooI2CHIDDevice_h
#define VoodooI2C_VoodooI2CHIDDevice_h


#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOTimerEventSource.h>

#define __le16 UInt16
#define __le32 UInt32

#define HID_MAX_DESCRIPTOR_SIZE 4096

#define I2C_HID_PWR_ON 0x00
#define I2C_HID_PWR_SLEEP 0x01


class VoodooI2C;

class VoodooI2CHIDDevice : public IOService
{
    typedef IOService super;
    OSDeclareDefaultStructors(VoodooI2CHIDDevice);
    
protected:
    VoodooI2C* _controller;
    
public:
    virtual bool attach(IOService * provider, IOService* child);
    virtual void detach(IOService * provider);
    void stop(IOService* device);
    
    
    typedef struct {
        
        unsigned short addr;
        
        void* _dev;
        
        IOWorkLoop* workLoop;
        //IOCommandGate* commandGate;
        
        IOInterruptEventSource *interruptSource;
        
        IOACPIPlatformDevice* provider;
        
        IOTimerEventSource* timerSource;
        
        char* name;
        
        bool reading;
        
    } I2CDevice;
    
    I2CDevice* hid_device;
    
    union command {
        UInt8 data[0];
        struct cmd {
            __le16 reg;
            UInt8 reportTypeID;
            UInt8 opcode;
        } c;
    };
    
    struct i2c_hid_desc {
        __le16 wHIDDescLength;
        __le16 bcdVersion;
        __le16 wReportDescLength;
        __le16 wReportDescRegister;
        __le16 wInputRegister;
        __le16 wMaxInputLength;
        __le16 wOutputRegister;
        __le16 wMaxOutputLength;
        __le16 wCommandRegister;
        __le16 wDataRegister;
        __le16 wVendorID;
        __le16 wProductID;
        __le16 wVersionID;
        __le32 reserved;
    } __packed;
    
    struct i2c_hid_platform_data {
        UInt16 hid_descriptor_address;
    };
    
    typedef struct {
        I2CDevice *client;
        
        
        union {
            UInt8 hdesc_buffer[sizeof(struct i2c_hid_desc)];
            struct i2c_hid_desc hdesc;
        };
        
        __le16 wHIDDescRegister;
        
        UInt bufsize;
        char *inbuf;
        char *rawbuf;
        char *cmdbuf;
        char *argsbuf;
        
        unsigned long flags;
        
        struct i2c_hid_platform_data pdata;
    } i2c_hid;
    
    i2c_hid *ihid;
    
    struct i2c_hid_cmd {
        UInt registerIndex;
        UInt8 opcode;
        UInt length;
        bool wait;
    };
    
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
    struct i2c_hid_cmd hid_reset_cmd = { I2C_HID_CMD(0x01),
                                        .wait = true
    };
    
    struct i2c_hid_cmd hid_descr_cmd = { .length = 2};
    
    struct i2c_hid_cmd hid_input_cmd = { .length = 2};
    
    struct i2c_hid_cmd hid_report_desc_cmd = {
        .registerIndex = offsetof(struct i2c_hid_desc, wReportDescRegister),
        .opcode = 0x00,
        .length =2
    };
    
    struct i2c_hid_cmd hid_set_power_cmd = { I2C_HID_CMD(0x08) };
    
    
    int initHIDDevice(I2CDevice *hid_device);
    
    int i2c_hid_acpi_pdata(i2c_hid *ihid);
    
    int i2c_hid_alloc_buffers(i2c_hid *ihid, UInt report_size);
    
    void i2c_hid_free_buffers(i2c_hid *ihid, UInt report_size);
    
    int i2c_hid_fetch_hid_descriptor(i2c_hid *ihid);
    
    int i2c_hid_command(i2c_hid *ihid, struct i2c_hid_cmd *command, unsigned char *buf_recv, int data_len);
    
    int __i2c_hid_command(i2c_hid *ihid, struct i2c_hid_cmd *command, UInt8 reportID, UInt8 reportType, UInt8 *args, int args_len, unsigned char *buf_recv, int data_len);
    
    int i2c_hid_set_power(i2c_hid *ihid, int power_state);
    
    bool probe(IOService* device);
    
    void InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    
    void i2c_hid_get_input(OSObject* owner, IOTimerEventSource* sender);
    
    bool i2c_hid_get_report_descriptor(i2c_hid *ihid);
    
    int i2c_get_slave_address(I2CDevice* hid_device);
    
    bool i2c_hid_hwreset(i2c_hid *ihid);

};


#endif
