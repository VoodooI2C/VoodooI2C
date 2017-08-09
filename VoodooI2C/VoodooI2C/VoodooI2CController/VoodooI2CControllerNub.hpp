//
//  VoodooI2CControllerNub.hpp
//  VoodooI2C
//
//  Created by Alexandre on 03/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CControllerNub_hpp
#define VoodooI2CControllerNub_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOCommandGate.h>

class VoodooI2CController;
class VoodooI2CControllerDriver;

class VoodooI2CControllerNub : public IOService {
    OSDeclareDefaultStructors(VoodooI2CControllerNub);

 public:
    // data members

    VoodooI2CController* controller;
    VoodooI2CControllerDriver* driver;
    const char* name;
    IOCommandGate* command_gate;
    IOInterruptEventSource* interrupt_source;
    IOWorkLoop* work_loop;

    // function members

    bool attach(IOService* provider);
    void detach(IOService* provider);
    IOReturn getACPIParams(const char* method, UInt32* hcnt, UInt32* lcnt, UInt32* sda_hold);
    UInt32 readRegister(int offset);
    bool start(IOService* provider);
    void stop(IOService* provider);
    void writeRegister(UInt32 value, int offset);

 private:
    void interruptOccured(OSObject* owner, IOInterruptEventSource* src, int intCount);
    void releaseResources();
};

#endif /* VoodooI2CControllerNub_hpp */
