//
//  VoodooCyapaGen3Device.hpp
//  VoodooI2C
//
//  Created by CoolStar on 12/13/15.
//  Copyright © 2015 CoolStar. All rights reserved.
//  ported from crostrackpad 3.0 beta 3 for Windows
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

#define __le16 UInt16
#define __le32 UInt32

struct csgesture_softc {
    //hardware input
    int x[15];
    int y[15];
    int p[15];
    
    bool buttondown;
    
    //system output
    int dx;
    int dy;
    
    int scrollx;
    int scrolly;
    
    int buttonmask;
    
    //used internally in driver
    bool mouseDownDueToTap;
    int idForMouseDown;
    bool mousedown;
    int mousebutton;
    
    int lastx[15];
    int lasty[15];
    int lastp[15];
    
    int xhistory[15][10];
    int yhistory[15][10];
    
    int flextotalx[15];
    int flextotaly[15];
    
    int totalx[15];
    int totaly[15];
    int totalp[15];
    
    int multitaskingx;
    int multitaskingy;
    int multitaskinggesturetick;
    bool multitaskingdone;
    
    int tick[15];
    int truetick[15];
    int ticksincelastrelease;
    int tickssinceclick;
};

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

class VoodooI2C;
class VoodooHIDWrapper;
class IOBufferMemoryDescriptor;

class VoodooI2CCyapaGen3Device : public IOService
{
    typedef IOService super;
    OSDeclareDefaultStructors(VoodooI2CCyapaGen3Device);
    
private:
    VoodooHIDWrapper* _wrapper;
    
    void initialize_wrapper(void);
    void destroy_wrapper(void);
    
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
    
    struct csgesture_softc softc;
    
    int initHIDDevice(I2CDevice *hid_device);
    
    bool probe(IOService* device);
    
    void InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    
    void get_input(OSObject* owner, IOTimerEventSource* sender);
    
    void write_report_descriptor_to_buffer(IOBufferMemoryDescriptor *buffer);
    
    int i2c_get_slave_address(I2CDevice* hid_device);
    
    SInt32 readI2C(uint8_t reg, size_t len, uint8_t *values);
    SInt32 writeI2C(uint8_t reg, size_t len, uint8_t *values);
    
    void update_relative_mouse(uint8_t button,
                          uint8_t x, uint8_t y, uint8_t wheelPosition, uint8_t wheelHPosition);
    
    int distancesq(int delta_x, int delta_y);
    void ProcessMove(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    void ProcessScroll(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    void TapToClick(csgesture_softc *sc, int button);
    void ProcessGesture(csgesture_softc *sc);
    void TrackpadRawInput(struct csgesture_softc *sc, cyapa_regs *regs, int tickinc);
};


#endif
