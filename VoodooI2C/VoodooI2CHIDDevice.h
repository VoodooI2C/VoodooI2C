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
#include "VoodooI2CDevice.h"

#define __le16 UInt16
#define __le32 UInt32

#define HID_MAX_DESCRIPTOR_SIZE 4096

#define I2C_HID_PWR_ON 0x00
#define I2C_HID_PWR_SLEEP 0x01


class VoodooI2C;
class VoodooHIDWrapper;
class IOBufferMemoryDescriptor;

class VoodooI2CHIDDevice : public VoodooI2CDevice
{
    typedef IOService super;
    OSDeclareDefaultStructors(VoodooI2CHIDDevice);

private:
    VoodooHIDWrapper* _wrapper;

    void initialize_wrapper(void);
    void destroy_wrapper(void);

protected:
    VoodooI2C* _controller;
    
public:
    virtual bool attach(IOService * provider, IOService* child) override;
    virtual void detach(IOService * provider) override;
    void stop(IOService* device) override;
    
    struct __attribute__((__packed__)) i2c_hid_descr {
        uint16_t wHIDDescLength;
        uint16_t bcdVersion;
        uint16_t wReportDescLength;
        uint16_t wReportDescRegister;
        uint16_t wInputRegister;
        uint16_t wMaxInputLength;
        uint16_t wOutputRegister;
        uint16_t wMaxOutputLength;
        uint16_t wCommandRegister;
        uint16_t wDataRegister;
        uint16_t wVendorID;
        uint16_t wProductID;
        uint16_t wVersionID;
        uint32_t reserved;
    };
    
    typedef struct {
        
        unsigned short addr;
        
        void* _dev;
        
        IOWorkLoop* workLoop;
        
        IOInterruptEventSource *interruptSource;
        
        IOACPIPlatformDevice* provider;
        
        char* name;
        
        bool reading;
        
        bool deviceIsAwake;
        
        UInt16 hid_descriptor_address;
        
        struct i2c_hid_descr hdesc;
        
        uint8_t *rdesc;
        
        uint16_t rsize;
        
    } I2CDevice;
    
    I2CDevice* hid_device;
    
    struct i2c_msg {
        UInt16 addr;
        UInt16 flags;
        UInt16 len;
        UInt8 *buf;
        
#define I2C_M_TEN 0x0010
#define I2C_M_RD 0x0001
#define I2C_M_RECV_LEN 0x0400
    };
    
    int fetch_hid_descriptor();
    int fetch_report_descriptor();
    
    int set_power(int power_state);
    int reset_dev();
    
    int initHIDDevice(I2CDevice *hid_device);
    
    bool probe(IOService* device);
    
    void InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    
    virtual IOReturn setPowerState(unsigned long powerState, IOService *whatDevice) override;
    
    int i2c_hid_descriptor_address(I2CDevice *hid_device);
    int i2c_get_slave_address(I2CDevice* hid_device);
    
    SInt32 readI2C(uint8_t *values, size_t len);
    SInt32 writeI2C(uint8_t *values, size_t len);
    
    void get_input(OSObject* owner, IOTimerEventSource* sender);
    
    void write_report_descriptor_to_buffer(IOBufferMemoryDescriptor *buffer);
};


#endif
