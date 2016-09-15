#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOCommandGate.h>
#include <string.h>
#include "VoodooI2CDevice.h"
#include "VoodooI2CHIDDevice.h"
#include "VoodooCyapaGen3Device.h"
#include "VoodooElanTouchpadDevice.h"
#include "VoodooI2CAtmelMxtScreenDevice.h"

#define kIOPMPowerOff		0

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

#define STATUS_IDLE 0x0
#define STATUS_WRITE_IN_PROGRESS 0x1
#define STATUS_READ_IN_PROGRESS 0x2

#define TIMEOUT 20

#define DW_IC_CON_MASTER 0x1
#define DW_IC_CON_SPEED_STD 0x2
#define DW_IC_CON_SPEED_FAST 0x4
#define DW_IC_CON_10BITADDR_MASTER 0x10
#define DW_IC_CON_RESTART_EN 0x20
#define DW_IC_CON_SLAVE_DISABLE 0x40

#define I2C_FUNC_I2C 0x00000001
#define I2C_FUNC_10BIT_ADDR 0x00000002
#define I2C_FUNC_SMBUS_READ_BYTE 0x00020000
#define I2C_FUNC_SMBUS_WRITE_BYTE 0x00040000
#define I2C_FUNC_SMBUS_READ_BYTE_DATA 0x00080000
#define I2C_FUNC_SMBUS_WRITE_BYTE_DATA 0x00100000
#define I2C_FUNC_SMBUS_READ_WORD_DATA 0x00200000
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA 0x00400000
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK 0x04000000
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK 0x08000000

#define I2C_FUNC_SMBUS_BYTE (I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE)
#define I2C_FUNC_SMBUS_BYTE_DATA (I2C_FUNC_SMBUS_READ_BYTE_DATA | I2C_FUNC_SMBUS_WRITE_BYTE_DATA)
#define I2C_FUNC_SMBUS_WORD_DATA (I2C_FUNC_SMBUS_READ_WORD_DATA | I2C_FUNC_SMBUS_WRITE_WORD_DATA)
#define I2C_FUNC_SMBUS_I2C_BLOCK (I2C_FUNC_SMBUS_READ_I2C_BLOCK | I2C_FUNC_SMBUS_WRITE_I2C_BLOCK)



#define DW_IC_CON 0x0
#define DW_IC_TAR 0x4
#define DW_IC_SAR 0x8
#define DW_IC_DATA_CMD 0x10
#define DW_IC_SS_SCL_HCNT 0x14
#define DW_IC_SS_SCL_LCNT 0x18
#define DW_IC_FS_SCL_HCNT 0x1c
#define DW_IC_FS_SCL_LCNT 0x20
#define DW_IC_INTR_STAT 0x2c
#define DW_IC_INTR_MASK 0x30
#define DW_IC_RAW_INTR_STAT 0x34
#define DW_IC_RX_TL 0x38
#define DW_IC_TX_TL 0x3c
#define DW_IC_CLR_INTR 0x40
#define DW_IC_CLR_RX_UNDER 0x44
#define DW_IC_CLR_RX_OVER 0x48
#define DW_IC_CLR_TX_OVER 0x4c
#define DW_IC_CLR_RD_REQ 0x50
#define DW_IC_CLR_TX_ABRT 0x54
#define DW_IC_CLR_RX_DONE 0x58
#define DW_IC_CLR_ACTIVITY 0x5c
#define DW_IC_CLR_STOP_DET 0x60
#define DW_IC_CLR_START_DET 0x64
#define DW_IC_CLR_GEN_CALL 0x68
#define DW_IC_ENABLE 0x6c
#define DW_IC_STATUS 0x70
#define DW_IC_TXFLR 0x74
#define DW_IC_RXFLR 0x78
#define DW_IC_SDA_HOLD 0x7c
#define DW_IC_TX_ABRT_SOURCE 0x80
#define DW_IC_DMA_CR 0x88
#define DW_IC_DMA_TDLR 0x8c
#define DW_IC_DMA_RDLR 0x90
#define DW_IC_SDA_SETUP 0x94
#define DW_IC_ENABLE_STATUS 0x9c
#define DW_IC_FS_SPKLEN 0xA0
#define DW_IC_COMP_PARAM_1 0xf4
#define DW_IC_COMP_VERSION 0xf8
#define DW_IC_SDA_HOLD_MIN_VERS 0x3131312A
#define DW_IC_COMP_TYPE 0xfc
#define DW_IC_COMP_TYPE_VALUE 0x44570140

#define DW_IC_STATUS_ACTIVITY 0x1

#define DW_IC_ERR_TX_ABRT 0x1

#define DW_IC_CON_10BITADDR_MASTER 0x10

#define DW_IC_INTR_RX_UNDER 0x001
#define DW_IC_INTR_RX_OVER 0x002
#define DW_IC_INTR_RX_FULL 0x004
#define DW_IC_INTR_TX_OVER 0x008
#define DW_IC_INTR_TX_EMPTY 0x010
#define DW_IC_INTR_RD_REQ 0x020
#define DW_IC_INTR_TX_ABRT 0x040
#define DW_IC_INTR_RX_DONE 0x080
#define DW_IC_INTR_ACTIVITY 0x100
#define DW_IC_INTR_STOP_DET 0x200
#define DW_IC_INTR_START_DET 0x400
#define DW_IC_INTR_GEN_CALL 0x800

#define DW_IC_INTR_DEFAULT_MASK     (DW_IC_INTR_RX_FULL | \
                                     DW_IC_INTR_TX_EMPTY | \
                                     DW_IC_INTR_TX_ABRT | \
                                     DW_IC_INTR_STOP_DET | \
                                     DW_IC_INTR_START_DET)


#define BIT(nr) (1UL << (nr))
#define DW_IC_TAR_10BITADDR_MASTER BIT(12);

#define I2C_SMBUS_BLOCK_MAX 32

#define I2C_SMBUS_QUICK 0
#define I2C_SMBUS_BYTE 1
#define I2C_SMBUS_BYTE_DATA 2
#define I2C_SMBUS_WORD_DATA 3
#define I2C_SMBUS_PROC_CALL 4
#define I2C_SMBUS_BLOCK_DATA 5
#define I2C_SMBUS_I2C_BLOCK_BROKEN 6
#define I2C_SMBUS_BLOCK_PROC_CALL 7
#define I2C_SMBUS_I2C_BLOCK_DATA 8


#define I2C_SMBUS_READ 1
#define I2C_SMBUS_WRITE 0

#define EAGAIN 35

#define RMI_PAGE_SELECT_REGISTER 0xff
#define RMI_I2C_PAGE(addr) (((addr) >> 8) & 0x0ff)

#define PDT_START_SCAN_LOCATION 0x00e9
#define PDT_END_SCAN_LOCATION 0x0005

#define RMI4_END_OF_PDT(id) ((id) == 0x00 || (id) == 0xff)
#define RMI4_MAX_PAGE 0xff
#define RMI4_PAGE_SIZE 0x100

#define RMI_PRODUCT_ID_LENGTH 10

#define __le16 SInt16
#define __le32 SInt32

#define HID_MIN_BUFFER_SIZE 64

#define I2C_HID_PWR_ON 0x00
#define I2C_HID_PWR_SLEEP 0x01

enum VoodooI2CDeviceMode {
    VoodooI2CDeviceModeACPI,
    VoodooI2CDeviceModePCI
};

class VoodooI2C : public IOService {
    
    OSDeclareDefaultStructors(VoodooI2C);
    
    
public:
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
    

    
    typedef struct {
        UInt8 byte;
        UInt16 word;
        UInt8 block[I2C_SMBUS_BLOCK_MAX + 2];
    } i2c_smbus_data;
    
    typedef struct {
        IOService *provider;
        
        VoodooI2CDeviceMode deviceMode;
        
        IOWorkLoop *workLoop;
        IOInterruptEventSource *interruptSource;
        
        IOCommandGate *commandGate;
        
        IOMemoryMap *mmap;
        IOVirtualAddress mmio;
        
        UInt32 clk_rate_khz;
        UInt32 sda_hold_time;
        UInt32 sda_falling_time;
        UInt32 scl_falling_time;
        
        UInt32 functionality;
        UInt32 master_cfg;
        
        UInt tx_fifo_depth;
        UInt rx_fifo_depth;
        
        UInt32 ss_hcnt;
        UInt32 ss_lcnt;
        
        UInt32 fs_hcnt;
        UInt32 fs_lcnt;
    
        struct i2c_algorithm *algo;
        
        bool commandComplete = false;
        struct i2c_msg *msgs;
        int msgs_num;
        int cmd_err;
        int msg_write_idx;
        int msg_read_idx;
        int msg_err;
        UInt status;
        UInt32 abort_source;
        int rx_outstanding;
        
        UInt32 rx_buf_len;
        UInt8 *rx_buf;
        UInt32 tx_buf_len;
        UInt8 *tx_buf;
        
        int retries = 5;
        char *name;
        
        bool ready;
        
        bool busIsAwake;
        
    } I2CBus;
    
    I2CBus* _dev;
    
    typedef struct {
        I2CBus *phys;
        UInt16 addr;
        char read_write;
        UInt8 command;
        int size;
        i2c_smbus_data *data;
    } commandGateTransaction;
    
    
    
    VoodooI2CDevice* bus_devices[2];
    int bus_devices_number;
    

    
    
    static bool getACPIParams(IOACPIPlatformDevice* fACPIDevice, char method[], UInt32 *hcnt, UInt32 *lcnt, UInt32 *sda_hold);
    bool acpiConfigure(I2CBus* _dev);
    bool fallbackConfigure(I2CBus *_dev);
    void disableI2CInt(I2CBus* _dev);
    void enableI2CDevice(I2CBus*, bool enabled);
    UInt32 funcI2C(I2CBus* _dev);
    virtual char* getMatchedName(IOService* provider);
    int handleTxAbortI2C(I2CBus* _dev);
    bool initI2CBus(I2CBus* _dev);
    virtual bool mapI2CMemory(I2CBus* _dev);
    void readI2C(I2CBus* _dev);
    UInt32 readl(I2CBus* _dev, int offset);
    UInt32 readClearIntrbitsI2C(I2CBus* _dev);
    void releaseAllI2CChildren();
    void setI2CPowerState(I2CBus* _dev, bool enabled);
    IOReturn setPowerState(unsigned long powerState, IOService *whatDevice);
    virtual bool start(IOService* provider);
    virtual void stop(IOService* provider);
    int waitBusNotBusyI2C(I2CBus* _dev);
    void writel(I2CBus* _dev, UInt32 b, int offset);
    int xferI2C(I2CBus* _dev, i2c_msg *msgs, int num);
    void xferInitI2C(I2CBus* _dev);
    void xferMsgI2C(I2CBus* _dev);
    
    int i2c_master_recv(VoodooI2CHIDDevice::I2CDevice I2CDevice, UInt8 *buf, int count);
    int i2c_master_send(VoodooI2CHIDDevice::I2CDevice I2CDevice, UInt8 *buf, int count);
    
    void interruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    
    //static I2CBus* getBusByName(char* name );
    
    SInt32 i2c_smbus_xfer(I2CBus *phys,
                                   UInt16 addr, char read_write, UInt8 command, int size,
                                   i2c_smbus_data *data);
    SInt32 i2c_smbus_xfer_gated(commandGateTransaction *transaction);
    
    SInt32 i2c_smbus_write_byte_data(I2CBus *phys, UInt16 addr, UInt8 command, UInt8 value);
    
    SInt32 i2c_smbus_read_i2c_block_data(I2CBus* phys, UInt16 addr, UInt8 command, UInt8 length, UInt8 *values);
    SInt32 i2c_smbus_write_i2c_block_data(I2CBus* phys, UInt16 addr, UInt8 command, UInt8 length, const UInt8 *values);
    
    int i2c_transfer(i2c_msg *msgs, int num);
    int i2c_transfer_gated(I2CBus* phys, i2c_msg *msgs, int *num);
    int __i2c_transfer(I2CBus* phys, i2c_msg *msgs, int num);
    //static int i2c_smbus_write_i2c_block_data;
    
    //static char* getName();
    
    void registerDump(I2CBus* _dev);
    
    void clearI2CInt(I2CBus* _dev);
    
    
};