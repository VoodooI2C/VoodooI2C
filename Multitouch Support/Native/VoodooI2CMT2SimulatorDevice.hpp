//
//  VoodooI2CMT2SimulatorDevice.hpp
//  VoodooI2C
//
//  Created by Alexandre on 10/02/2018.
//  Copyright © 2018 Alexandre Daoud and Kishor Prins. All rights reserved.
//

#ifndef VoodooI2CMT2SimulatorDevice_hpp
#define VoodooI2CMT2SimulatorDevice_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#include <IOKit/hid/IOHIDDevice.h>

#include <kern/clock.h>

#include "../../Dependencies/helpers.hpp"
#include "../MultitouchHelpers.hpp"
#include "../VoodooI2CDigitiserTransducer.hpp"
#include "../VoodooI2CMultitouchInterface.hpp"

#define MT2_MAX_X 8134
#define MT2_MAX_Y 5206

/* Finger Packet
 +---+---+---+---+---+---+---+---+---+
 |   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 +---+---+---+---+---+---+---+---+---+
 | 0 |           x: SInt13           |
 +---+-----------+                   +
 | 1 |           |                   |
 +---+           +-------------------+
 | 2 |           y: SInt13           |
 +---+-----------+-----------+       +
 | 3 |   state   |  priority |       |
 |   |   UInt3   |   UInt3   |       |
 +---+-----------+-----------+-------+
 | 4 |       touchMajor: UInt8       |
 +---+-------------------------------+
 | 5 |       touchMinor: UInt8       |
 +---+-------------------------------+
 | 6 |          size: UInt8          |
 +---+-------------------------------+
 | 7 |        pressure: UInt8        |
 +---+-----------+---+---------------+
 | 8 |   angle   | 0 |   fingerID    |
 |   |   UInt3   |   |     UInt4     |
 +---+-----------+---+---------------+
 */

struct __attribute__((__packed__)) MAGIC_TRACKPAD_INPUT_REPORT_FINGER {
    SInt16 X: 13;
    SInt16 Y: 13;
    UInt8 Priority: 3;
    UInt8 State: 3;
    UInt8 Touch_Major;
    UInt8 Touch_Minor;
    UInt8 Size;
    UInt8 Pressure;
    UInt8 Identifier: 4;
    UInt8 : 1;
    UInt8 Angle: 3;
};

struct __attribute__((__packed__)) MAGIC_TRACKPAD_INPUT_REPORT {
    UInt8 ReportID;
    UInt8 Button;
    UInt8 Unused[5];
    
    UInt8 TouchActive;
    
    UInt8 multitouch_report_id;
    UInt8 timestamp_buffer[3];
    
    MAGIC_TRACKPAD_INPUT_REPORT_FINGER FINGERS[12]; // May support more fingers
};

class VoodooI2CNativeEngine;

class EXPORT VoodooI2CMT2SimulatorDevice : public IOHIDDevice {
    OSDeclareDefaultStructors(VoodooI2CMT2SimulatorDevice);
    
public:
    void constructReport(VoodooI2CMultitouchEvent multitouch_event, AbsoluteTime timestamp);
    IOReturn setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options);

    IOReturn getReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options);
    IOReturn newReportDescriptor(IOMemoryDescriptor** descriptor) const override;
    OSNumber* newVendorIDNumber() const override;
    
    
    OSNumber* newProductIDNumber() const override;
    
    
    OSNumber* newVersionNumber() const override;
    
    
    OSString* newTransportString() const override;
    
    
    OSString* newManufacturerString() const override;
    
    OSNumber* newPrimaryUsageNumber() const override;
    
    OSNumber* newPrimaryUsagePageNumber() const override;
    
    OSString* newProductString() const override;
    
    OSString* newSerialNumberString() const override;
    
    OSNumber* newLocationIDNumber() const override;
    
    IOReturn setPowerState(unsigned long whichState, IOService* whatDevice);
    
    bool start(IOService* provider);

    void stop(IOService* provider);
    
    void releaseResources();

private:
    bool ready_for_reports = false;
    VoodooI2CNativeEngine* engine;
    AbsoluteTime start_timestamp;
    OSData* new_get_report_buffer = NULL;
    UInt64 touch_state[15];
    int stylus_check = 0;
    IOWorkLoop* work_loop;
    IOCommandGate* command_gate;
    MAGIC_TRACKPAD_INPUT_REPORT input_report;

    void constructReportGated(VoodooI2CMultitouchEvent& multitouch_event, AbsoluteTime& timestamp);
};


#endif /* VoodooI2CMT2SimulatorDevice_hpp */
