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

#include "VoodooI2CMT2PointingWrapper.hpp"

struct __attribute__((__packed__)) MAGIC_TRACKPAD_INPUT_REPORT_FINGER {
    UInt8 AbsX;
    UInt8 AbsXY;
    UInt8 AbsY[2];
    UInt8 Touch_Major;
    UInt8 Touch_Minor;
    UInt8 Size;
    UInt8 Pressure;
    UInt8 Orientation_Origin;
};

struct __attribute__((__packed__)) MAGIC_TRACKPAD_INPUT_REPORT {
    UInt8 ReportID;
    UInt8 Button;
    UInt8 Unused[5];
    
    UInt8 TouchActive;
    
    UInt8 multitouch_report_id;
    UInt8 timestamp_buffer[3];
    
    MAGIC_TRACKPAD_INPUT_REPORT_FINGER FINGERS[12]; //May support more fingers
};

class VoodooI2CNativeEngine;

class VoodooI2CMT2SimulatorDevice : public IOHIDDevice {
    OSDeclareDefaultStructors(VoodooI2CMT2SimulatorDevice);
    
public:
    void constructReport(VoodooI2CMultitouchEvent multitouch_event, AbsoluteTime timestamp);
    IOReturn setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options);
    
    IOReturn getReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options);
    virtual IOReturn newReportDescriptor(IOMemoryDescriptor** descriptor) const override;
    virtual OSNumber* newVendorIDNumber() const override;
    
    
    virtual OSNumber* newProductIDNumber() const override;
    
    
    virtual OSNumber* newVersionNumber() const override;
    
    
    virtual OSString* newTransportString() const override;
    
    
    virtual OSString* newManufacturerString() const override;
    
    virtual OSNumber* newPrimaryUsageNumber() const override;
    
    virtual OSNumber* newPrimaryUsagePageNumber() const override;
    
    virtual OSString* newProductString() const override;
    
    virtual OSString* newSerialNumberString() const override;
    
    virtual OSNumber* newLocationIDNumber() const override;
    
    IOReturn setPowerState(unsigned long whichState, IOService* whatDevice);
    
    bool start(IOService* provider);

    void stop(IOService* provider);
    
    void releaseResources();
protected:
private:
    bool ready_for_reports = false;
    VoodooI2CNativeEngine* engine;
    AbsoluteTime start_timestamp;
    OSData* new_get_report_buffer;
    UInt16 stashed_unknown[15];
    UInt8 touch_state[15];
    UInt8 new_touch_state[15];
    int last_finger_count;
    IOWorkLoop* work_loop;
    IOCommandGate* command_gate;
    MAGIC_TRACKPAD_INPUT_REPORT input_report;
    VoodooI2CMT2PointingWrapper* pointing_wrapper;

    int factor_x;
    int factor_y;
    
    void constructReportGated(VoodooI2CMultitouchEvent& multitouch_event, AbsoluteTime& timestamp);
};


#endif /* VoodooI2CMT2SimulatorDevice_hpp */
