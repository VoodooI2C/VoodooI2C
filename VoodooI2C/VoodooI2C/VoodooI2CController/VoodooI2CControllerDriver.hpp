//
//  VoodooI2CControllerDriver.hpp
//  VoodooI2C
//
//  Created by Alexandre on 03/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CControllerDriver_hpp
#define VoodooI2CControllerDriver_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>

#include "VoodooI2CControllerConstants.hpp"
#include "VoodooI2CControllerNub.hpp"
#include "../VoodooI2CDevice/VoodooI2CDeviceNub.hpp"
#include "../../../Miscellaneous/helpers.hpp"

// The default values in the following struct are for Haswell PCI as
// specified in Linux::i2c-designware-pcidrv.c

typedef struct {
    UInt16 address;
    UInt8 *buffer;
    UInt16 flags;
    UInt16 length;
} VoodooI2CControllerBusMessage;

typedef struct {
    UInt32 ss_hcnt = 0x01b0;
    UInt32 fs_hcnt = 0x48;
    UInt32 ss_lcnt = 0x01fb;
    UInt32 fs_lcnt = 0xa0;
    UInt32 sda_hold = 0x9;
} VoodooI2CControllerBusConfig;

typedef struct {
    UInt32 abort_source;
    VoodooI2CControllerBusConfig* acpi_config;
    bool awake;
    UInt32 bus_config;
    int command_error;
    bool command_complete = false;
    UInt32 functionality;
    VoodooI2CControllerBusMessage* messages;
    int message_error;
    int message_number;
    int message_read_index;
    int message_write_index;
    const char* name;
    UInt32 receive_buffer_length;
    UInt8 *receive_buffer;
    UInt receive_fifo_depth;
    int receive_outstanding;
    UInt status;
    UInt32 transaction_buffer_length;
    UInt8 *transaction_buffer;
    UInt transaction_fifo_depth;
} VoodooI2CControllerBusDevice;

class VoodooI2CController;

class VoodooI2CControllerDriver : public IOService {
  OSDeclareDefaultStructors(VoodooI2CControllerDriver);

 public:
    // data members

    VoodooI2CControllerBusDevice* bus_device;
    OSArray* device_nubs;
    VoodooI2CControllerNub* nub;

    // function members

    void free();
    void handleInterrupt();
    bool init(OSDictionary* properties);
    VoodooI2CControllerDriver* probe(IOService* provider, SInt32* score);
    bool start(IOService* provider);
    void stop(IOService* provider);

 private:
    IOReturn getBusConfig();
    void handleAbortI2C();
    IOReturn initialiseBus();
    IOReturn prepareTransferI2C(VoodooI2CControllerBusMessage* messages, int* number);
    IOReturn publishNubs();
    UInt32 readClearInterruptBits();
    void readFromBus();
    void requestTransferI2C();
    IOReturn setPowerState(unsigned long whichState, IOService* whatDevice);
    IOReturn toggleBusState(VoodooI2CState enabled);
    inline void toggleClockGating(VoodooI2CState enabled);
    void toggleInterrupts(VoodooI2CState enabled);
    IOReturn transferI2C(VoodooI2CControllerBusMessage* messages, int number);
    IOReturn transferI2CGated(VoodooI2CControllerBusMessage* messages, int* number);
    void transferMessageToBus();
    IOReturn waitBusNotBusyI2C();
};


#endif /* VoodooI2CControllerDriver_hpp */
