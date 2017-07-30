//
//  VoodooPrecisionTouchpadDevice.h
//  VoodooI2C
//
//  Created by CoolStar on 7/29/17.
//  Copyright © 2016 CoolStar. All rights reserved.
//  ported from crostrackpad-elan 3.0 for Windows
//  based off VoodooElanTouchpadDevice
//

/*
 * NOTE: THIS IS A TEMPLATE DEVICE. IT HAS STRUCTS AND REPORT ID'S THAT
 * NEED TO BE ADJUSTED FOR YOUR TOUCHPAD. IF YOU DO NOT HAVE AN ELAN0651
 * YOU NEED TO MODIFY THIS DRIVER BEFORE USING IT.
 */

#ifndef VoodooI2C_VoodooPrecisionTouchpadDevice_h
#define VoodooI2C_VoodooPrecisionTouchpadDevice_h

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOTimerEventSource.h>
#include "VoodooI2CDevice.h"
#include "csgesture.h"

class VoodooI2C;

class VoodooI2CPrecisionTouchpadDevice : public VoodooI2CDevice
{
    typedef IOService super;
    OSDeclareDefaultStructors(VoodooI2CPrecisionTouchpadDevice);
    
private:
    CSGesture* _wrapper;
    
    uint32_t hw_res_x, hw_res_y;
    
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
        //IOCommandGate* commandGate;
        
        IOInterruptEventSource *interruptSource;
        
        IOACPIPlatformDevice* provider;
        
        IOTimerEventSource* timerSource;
        
        char* name;
        
        bool reading;

        bool trackpadIsAwake;
        
        uint8_t nullReportCount;
        
        UInt16 hid_descriptor_address;
        
        struct i2c_hid_descr hdesc;
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
    
    I2CDevice* hid_device;
    
    struct csgesture_softc softc;
    
    int fetch_hid_descriptor();
    
    int set_power(int power_state);
    int reset_dev();
    
    int write_feature(uint8_t reportID, uint8_t *buf, size_t buf_len);
    
    void enable_abs();
    
    int initHIDDevice(I2CDevice *hid_device);
    
    bool probe(IOService* device);
    
    void InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    
    void readInput(int runLoop);
    
    void get_input(OSObject* owner, IOTimerEventSource* sender);
    
    IOReturn setPowerState(unsigned long powerState, IOService *whatDevice) override;
    
    int i2c_hid_descriptor_address(I2CDevice *hid_device);
    int i2c_get_slave_address(I2CDevice* hid_device);
    
    SInt32 readI2C(uint8_t *values, size_t len);
    SInt32 writeI2C(uint8_t *values, size_t len);
    void TrackpadRawInput(struct csgesture_softc *sc, uint8_t *report, int tickinc);
};


#endif
