//
//  VoodooCyapaGen3Device.hpp
//  VoodooI2C
//
//  Created by CoolStar on 12/13/15.
//  Copyright © 2015 CoolStar. All rights reserved.
//  ported from crostrackpad 3.0 for Windows
//

#ifndef VoodooI2C_VoodooCyapaGen3Device_h
#define VoodooI2C_VoodooCyapaGen3Device_h

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

#define __le16 UInt16
#define __le32 UInt32

#define HID_MAX_DESCRIPTOR_SIZE 4096

#define CYAPA_MAX_MT    5

#define CYAPA_BOOT_BUSY		0x80
#define CYAPA_BOOT_RUNNING	0x10
#define CYAPA_BOOT_DATA_VALID	0x08
#define CYAPA_BOOT_CSUM_VALID	0x01

#define CYAPA_ERROR_INVALID     0x80
#define CYAPA_ERROR_INVALID_KEY 0x40
#define CYAPA_ERROR_BOOTLOADER	0x20
#define CYAPA_ERROR_CMD_CSUM    0x10
#define CYAPA_ERROR_FLASH_PROT  0x08
#define CYAPA_ERROR_FLASH_CSUM  0x04

#define CYAPA_STAT_RUNNING      0x80
#define CYAPA_STAT_PWR_MASK     0x0C
#define  CYAPA_PWR_OFF          0x00
#define  CYAPA_PWR_IDLE         0x08
#define  CYAPA_PWR_ACTIVE       0x0C

#define CYAPA_STAT_DEV_MASK     0x03
#define  CYAPA_DEV_NORMAL       0x03
#define  CYAPA_DEV_BUSY         0x01

#define CYAPA_FNGR_DATA_VALID   0x08
#define CYAPA_FNGR_MIDDLE       0x04
#define CYAPA_FNGR_RIGHT        0x02
#define CYAPA_FNGR_LEFT         0x01
#define CYAPA_FNGR_NUMFINGERS(c) (((c) >> 4) & 0x0F)

#define CYAPA_TOUCH_X(regs, i)  ((((regs)->touch[i].xy_high << 4) & 0x0F00) | \
(regs)->touch[i].x_low)
#define CYAPA_TOUCH_Y(regs, i)  ((((regs)->touch[i].xy_high << 8) & 0x0F00) | \
(regs)->touch[i].y_low)
#define CYAPA_TOUCH_P(regs, i)  ((regs)->touch[i].pressure)

#define CMD_BOOT_STATUS		0x00	/* only if in boot state */
#define CMD_DEV_STATUS          0x00	/* only if in operational state */
#define CMD_SOFT_RESET          0x28
#define CMD_POWER_MODE          0x29
#define  CMD_POWER_MODE_OFF	0x00
#define  CMD_POWER_MODE_IDLE	0x14
#define  CMD_POWER_MODE_FULL	0xFC
#define CMD_QUERY_CAPABILITIES  0x2A

typedef struct __attribute__((__packed__)){
    uint8_t stat;			/* CYAPA_STAT_xxx */
    uint8_t boot;			/* CYAPA_BOOT_xxx */
    uint8_t error;
} cyapa_boot_regs;

typedef struct __attribute__((__packed__)){
    uint8_t stat;
    uint8_t fngr;
    
    struct {
        uint8_t xy_high;        /* 7:4 high 4 bits of x */
        uint8_t x_low;          /* 3:0 high 4 bits of y */
        uint8_t y_low;
        uint8_t pressure;
        uint8_t id;             /* 1-15 incremented each touch */
    } touch[CYAPA_MAX_MT];
} cyapa_regs;

typedef struct __attribute__((__packed__)){
    uint8_t prod_ida[5];    /* 0x00 - 0x04 */
    uint8_t prod_idb[6];    /* 0x05 - 0x0A */
    uint8_t prod_idc[2];    /* 0x0B - 0x0C */
    uint8_t reserved[2];    /* 0x0D - 0x0E */
    uint8_t fw_maj_ver;     /* 0x0F */
    uint8_t fw_min_ver;		/* 0x10 */
    uint8_t reserved2[2];   /* 0x11 - 0x12 */
    uint8_t buttons;        /* 0x13 */
    uint8_t gen;            /* 0x14, low 4 bits */
    uint8_t max_abs_xy_high;/* 0x15 7:4 high x bits, 3:0 high y bits */
    uint8_t max_abs_x_low;  /* 0x16 */
    uint8_t max_abs_y_low;  /* 0x17 */
    uint8_t phy_siz_xy_high;/* 0x18 7:4 high x bits, 3:0 high y bits */
    uint8_t phy_siz_x_low;  /* 0x19 */
    uint8_t phy_siz_y_low;  /* 0x1A */
} cyapa_cap;

class VoodooI2C;

class VoodooI2CCyapaGen3Device : public VoodooI2CDevice
{
    typedef IOService super;
    OSDeclareDefaultStructors(VoodooI2CCyapaGen3Device);
    
private:
    CSGesture* _wrapper;
    
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
        //IOCommandGate* commandGate;
        
        IOInterruptEventSource *interruptSource;
        
        IOACPIPlatformDevice* provider;
        
        IOTimerEventSource* timerSource;
        
        char* name;
        
        bool reading;
        
        bool trackpadIsAwake;
        
    } I2CDevice;
    
    struct i2c_msg {
        UInt16 addr;
        UInt16 flags;
        UInt16 len;
        UInt8 *buf;
        
#define I2C_M_TEN 0x0010
#define I2C_M_RD 0x0001
#define I2C_M_RECV_LEN 0x0400
    };
    
    I2CDevice* hid_device;
    
    struct csgesture_softc softc;
    
    int initHIDDevice(I2CDevice *hid_device);
    
    bool probe(IOService* device);
    
    void InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    
    void get_input(OSObject* owner, IOTimerEventSource* sender);
    
    IOReturn setPowerState(unsigned long powerState, IOService *whatDevice) override;
    
    int i2c_get_slave_address(I2CDevice* hid_device);
    
    void cyapa_set_power_mode(uint8_t power_mode);
    
    SInt32 readI2C(uint8_t reg, size_t len, uint8_t *values);
    SInt32 writeI2C(size_t len, uint8_t *values);
    void TrackpadRawInput(struct csgesture_softc *sc, cyapa_regs *regs, int tickinc);
};


#endif
