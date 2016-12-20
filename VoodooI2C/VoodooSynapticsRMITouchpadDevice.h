//
//  VoodooSynapticsRMITouchpadDevice.h
//  VoodooI2C
//
//  Created by CoolStar on 12/19/16.
//  Copyright © 2016 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooSynapticsRMITouchpadDevice_h
#define VoodooSynapticsRMITouchpadDevice_h

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

#define RMI_MOUSE_REPORT_ID		0x01 /* Mouse emulation Report */
#define RMI_WRITE_REPORT_ID		0x09 /* Output Report */
#define RMI_READ_ADDR_REPORT_ID		0x0a /* Output Report */
#define RMI_READ_DATA_REPORT_ID		0x0b /* Input Report */
#define RMI_ATTN_REPORT_ID		0x0c /* Input Report */
#define RMI_SET_RMI_MODE_REPORT_ID	0x0f /* Feature Report */

/* flags */
#define RMI_READ_REQUEST_PENDING	0
#define RMI_READ_DATA_PENDING		1
#define RMI_STARTED			2

#define RMI_SLEEP_NORMAL		0x0
#define RMI_SLEEP_DEEP_SLEEP		0x1

/* device flags */
#define RMI_DEVICE			BIT(0)
#define RMI_DEVICE_HAS_PHYS_BUTTONS	BIT(1)

/*
 * retrieve the ctrl registers
 * the ctrl register has a size of 20 but a fw bug split it into 16 + 4,
 * and there is no way to know if the first 20 bytes are here or not.
 * We use only the first 12 bytes, so get only them.
 */
#define RMI_F11_CTRL_REG_COUNT		12

enum rmi_mode_type {
    RMI_MODE_OFF = 0,
    RMI_MODE_ATTN_REPORTS = 1,
    RMI_MODE_NO_PACKED_ATTN_REPORTS = 2,
};

struct rmi_function {
    unsigned page;			/* page of the function */
    uint16_t query_base_addr;		/* base address for queries */
    uint16_t command_base_addr;		/* base address for commands */
    uint16_t control_base_addr;		/* base address for controls */
    uint16_t data_base_addr;		/* base address for datas */
    unsigned int interrupt_base;	/* cross-function interrupt number
                                     * (uniq in the device)*/
    unsigned int interrupt_count;	/* number of interrupts */
    unsigned int report_size;	/* size of a report */
    unsigned long irq_mask;		/* mask of the interrupts
                                 * (to be applied against ATTN IRQ) */
};

#define RMI_PAGE(addr) (((addr) >> 8) & 0xff)

#define RMI4_MAX_PAGE 0xff
#define RMI4_PAGE_SIZE 0x0100

#define PDT_START_SCAN_LOCATION 0x00e9
#define PDT_END_SCAN_LOCATION	0x0005
#define RMI4_END_OF_PDT(id) ((id) == 0x00 || (id) == 0xff)

typedef struct __attribute__((__packed__)) pdt_entry {
    uint8_t query_base_addr : 8;
    uint8_t command_base_addr : 8;
    uint8_t control_base_addr : 8;
    uint8_t data_base_addr : 8;
    uint8_t interrupt_source_count : 3;
    uint8_t bits3and4 : 2;
    uint8_t function_version : 2;
    uint8_t bit7 : 1;
    uint8_t function_number : 8;
};

#define RMI_DEVICE_F01_BASIC_QUERY_LEN	11

class VoodooI2C;

class VoodooSynapticsRMITouchpadDevice : public VoodooI2CDevice
{
    typedef IOService super;
    OSDeclareDefaultStructors(VoodooSynapticsRMITouchpadDevice);
    
private:
    CSGesture* _wrapper;
    
    uint16_t max_x;
    uint16_t max_y;
    
    int page;
    
    unsigned long flags;
    
    struct rmi_function f01;
    struct rmi_function f11;
    struct rmi_function f30;
    
    unsigned int max_fingers;
    unsigned int x_size_mm;
    unsigned int y_size_mm;
    bool read_f11_ctrl_regs;
    uint8_t f11_ctrl_regs[RMI_F11_CTRL_REG_COUNT];
    
    unsigned int gpio_led_count;
    unsigned int button_count;
    unsigned long button_mask;
    unsigned long button_state_mask;
    
    unsigned long device_flags;
    unsigned long firmware_id;
    
    uint8_t f01_ctrl0;
    uint8_t interrupt_enable_mask;
    bool restore_interrupt_mask;
    
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
        
#define I2C_HID_READ_PENDING (1 << 2);
        
#define I2C_HID_CMD(opcode_) \
.opcode = opcode_, .length = 4,\
.registerIndex = offsetof(struct i2c_hid_desc, wCommandRegister)
    };
    
    I2CDevice* hid_device;
    
    struct csgesture_softc softc;
    
    int initHIDDevice(I2CDevice *hid_device);
    
    bool probe(IOService* device);
    
    void InterruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    
    void get_input(OSObject* owner, IOTimerEventSource* sender);
    
    IOReturn setPowerState(unsigned long powerState, IOService *whatDevice) override;
    
    int i2c_get_slave_address(I2CDevice* hid_device);
    
    int rmi_read_block(uint16_t addr, uint8_t *buf, const int len);
    int rmi_write_report(uint8_t *report, size_t report_size);
    int rmi_read(uint16_t addr, uint8_t *buf);
    int rmi_write_block(uint16_t addr, uint8_t *buf, const int len);
    int rmi_write(uint16_t addr, uint8_t *buf);
    void rmi_register_function(struct pdt_entry *pdt_entry, int page, unsigned interrupt_count);
    
    int rmi_scan_pdt();
    int rmi_populate_f01();
    int rmi_populate_f11();
    int rmi_populate_f30();
    int rmi_populate();
    
    int rmi_set_mode(uint8_t mode);
    
    int rmi_set_page(uint8_t _page);
    
    SInt32 readI2C(size_t len, uint8_t *values);
    SInt32 writeI2C(size_t len, uint8_t *values);
    
    void rmi_f11_process_touch(struct csgesture_softc *sc, int slot, uint8_t finger_state, uint8_t *touch_data);
    int rmi_f11_input(struct csgesture_softc *sc, uint8_t *rmiInput);
    int rmi_f30_input(struct csgesture_softc *sc, uint8_t irq, uint8_t *rmiInput, int size);
    void TrackpadRawInput(struct csgesture_softc *sc, uint8_t report[40], int tickinc);
};
#endif
