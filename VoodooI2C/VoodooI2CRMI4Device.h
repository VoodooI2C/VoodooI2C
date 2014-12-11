//
//  VoodooI2CDevice.h
//  VoodooI2C
//
//  Created by Alexandre on 10/12/2014.
//  Copyright (c) 2014 Alexandre Daoud. All rights reserved.
//

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include "VoodooI2C.h"

#define RMI_PAGE_SELECT_REGISTER 0xff
#define RMI_I2C_PAGE(addr) (((addr) >> 8) & 0x0ff)

class VoodooI2C;
class VoodooI2CRMI4Device : public IOService {
    
    friend class VoodooI2C;
    OSDeclareDefaultStructors(VoodooI2CRMI4Device);
    
public:
    
    typedef struct {
        IOLock *page_mutex;
        int page;
    } rmi_i2c_data;
    
    IOACPIPlatformDevice *fACPIDevice;
    
    typedef struct {
        
        IOACPIPlatformDevice* provider;
        
        IOWorkLoop *workLoop;
        IOInterruptEventSource *interruptSource;
    
        VoodooI2C::I2CBus *bus_provider;
        
        unsigned short addr;

        rmi_i2c_data *data;
        
        char* name;
    
    } I2CDevice;
    
    I2CDevice* _dev;
    
    bool init();
    bool start(IOService* provider);
    void stop();
    char* getMatchedName(IOService* provider);
    void interruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    
    bool attach(VoodooI2C* provider);
    
    int rmi_set_page(I2CDevice *phys, UInt page);
    
    int rmi_i2c_write_block(I2CDevice *phys, UInt16 addr, UInt8 *buf, int len);
    int rmi_i2c_write(I2CDevice *phys, UInt16 addr, UInt8);

};

