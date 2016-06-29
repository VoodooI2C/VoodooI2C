//
//  VoodooElanTouchpadDevice.h
//  VoodooI2C
//
//  Created by CoolStar on 12/13/15.
//  Copyright © 2015 CoolStar. All rights reserved.
//  ported from crostrackpad-elan 3.0 beta 9.4 for Windows
//

#ifndef VoodooI2C_VoodooElanTouchpadDevice_h
#define VoodooI2C_VoodooElanTouchpadDevice_h

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

/* Elan i2c commands */
#define ETP_I2C_RESET			0x0100
#define ETP_I2C_WAKE_UP			0x0800
#define ETP_I2C_SLEEP			0x0801
#define ETP_I2C_DESC_CMD		0x0001
#define ETP_I2C_REPORT_DESC_CMD		0x0002
#define ETP_I2C_STAND_CMD		0x0005
#define ETP_I2C_UNIQUEID_CMD		0x0101
#define ETP_I2C_FW_VERSION_CMD		0x0102
#define ETP_I2C_SM_VERSION_CMD		0x0103
#define ETP_I2C_XY_TRACENUM_CMD		0x0105
#define ETP_I2C_MAX_X_AXIS_CMD		0x0106
#define ETP_I2C_MAX_Y_AXIS_CMD		0x0107
#define ETP_I2C_RESOLUTION_CMD		0x0108
#define ETP_I2C_PRESSURE_CMD		0x010A
#define ETP_I2C_IAP_VERSION_CMD		0x0110
#define ETP_I2C_SET_CMD			0x0300
#define ETP_I2C_POWER_CMD		0x0307
#define ETP_I2C_FW_CHECKSUM_CMD		0x030F
#define ETP_I2C_IAP_CTRL_CMD		0x0310
#define ETP_I2C_IAP_CMD			0x0311
#define ETP_I2C_IAP_RESET_CMD		0x0314
#define ETP_I2C_IAP_CHECKSUM_CMD	0x0315
#define ETP_I2C_CALIBRATE_CMD		0x0316
#define ETP_I2C_MAX_BASELINE_CMD	0x0317
#define ETP_I2C_MIN_BASELINE_CMD	0x0318

#define ETP_I2C_REPORT_LEN		34
#define ETP_I2C_DESC_LENGTH		30
#define ETP_I2C_REPORT_DESC_LENGTH	158
#define ETP_I2C_INF_LENGTH		2
#define ETP_I2C_IAP_PASSWORD		0x1EA5
#define ETP_I2C_IAP_RESET		0xF0F0
#define ETP_I2C_MAIN_MODE_ON		(1 << 9)
#define ETP_I2C_IAP_REG_L		0x01
#define ETP_I2C_IAP_REG_H		0x06

#define ETP_ENABLE_ABS		0x0001
#define ETP_ENABLE_CALIBRATE	0x0002
#define ETP_DISABLE_CALIBRATE	0x0000
#define ETP_DISABLE_POWER	0x0001
#define ETP_PRESSURE_OFFSET	25

#define ETP_MAX_PRESSURE	255
#define ETP_FWIDTH_REDUCE	90
#define ETP_FINGER_WIDTH	15
#define ETP_RETRY_COUNT		3

#define ETP_MAX_FINGERS		5
#define ETP_FINGER_DATA_LEN	5
#define ETP_REPORT_ID		0x5D
#define ETP_REPORT_ID_OFFSET	2
#define ETP_TOUCH_INFO_OFFSET	3
#define ETP_FINGER_DATA_OFFSET	4
#define ETP_HOVER_INFO_OFFSET	30
#define ETP_MAX_REPORT_LEN	34

enum tp_mode {
    IAP_MODE = 1,
    MAIN_MODE
};

class VoodooI2C;
class VoodooElanTouchpadWrapper;
class IOBufferMemoryDescriptor;

class VoodooI2CElanTouchpadDevice : public VoodooI2CDevice
{
    typedef IOService super;
    OSDeclareDefaultStructors(VoodooI2CElanTouchpadDevice);
    
private:
    VoodooElanTouchpadWrapper* _wrapper;
    
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
    
    uint8_t prevreport[ETP_MAX_REPORT_LEN];
    
    I2CDevice* hid_device;
    
    struct csgesture_softc softc;
    
    int initHIDDevice(I2CDevice *hid_device);
    
    bool probe(IOService* device);
    
    void InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    
    void get_input(OSObject* owner, IOTimerEventSource* sender);
    
    int reportDescriptorLength();
    
    int vendorID();
    int productID();
    
    void write_report_to_buffer(IOMemoryDescriptor *buffer);
    void write_report_descriptor_to_buffer(IOMemoryDescriptor *buffer);
    
    IOReturn setPowerState(unsigned long powerState, IOService *whatDevice);
    
    int i2c_get_slave_address(I2CDevice* hid_device);
    
    void elan_i2c_read_cmd(uint16_t reg, uint8_t *val);
    void elan_i2c_write_cmd(uint16_t reg, uint16_t cmd);
    
    SInt32 readI2C(uint8_t reg, size_t len, uint8_t *values);
    SInt32 readI2C16(uint16_t reg, size_t len, uint8_t *values);
    SInt32 writeI2C(uint8_t reg, size_t len, uint8_t *values);
    SInt32 writeI2C16(uint16_t reg, size_t len, uint8_t *values);
    
    void update_relative_mouse(char button,
                               char x, char y, char wheelPosition, char wheelHPosition);
    void update_keyboard(uint8_t shiftKeys, uint8_t *keyCodes);
    
    int distancesq(int delta_x, int delta_y);
    bool ProcessMove(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    bool ProcessScroll(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    bool ProcessThreeFingerSwipe(csgesture_softc *sc, int abovethreshold, int iToUse[3]);
    
    void TapToClickOrDrag(csgesture_softc *sc, int button);
    void ClearTapDrag(csgesture_softc *sc, int i);
    void ProcessGesture(csgesture_softc *sc);
    void TrackpadRawInput(struct csgesture_softc *sc, uint8_t report[ETP_MAX_REPORT_LEN], int tickinc);
};


#endif
