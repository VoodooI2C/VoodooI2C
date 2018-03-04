//
//  VoodooI2CMT2SimulatorDevice.cpp
//  VoodooI2C
//
//  Created by Alexandre on 10/02/2018.
//  Copyright © 2018 Alexandre Daoud and Kishor Prins. All rights reserved.
//

#include "VoodooI2CMT2SimulatorDevice.hpp"
#include "VoodooI2CNativeEngine.hpp"

#include <IOKit/IOWorkLoop.h>
#include <IOKit/IOCommandGate.h>

#define super IOHIDDevice
OSDefineMetaClassAndStructors(VoodooI2CMT2SimulatorDevice, IOHIDDevice);

unsigned char report_descriptor[] = {0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x09, 0x01, 0xa1, 0x00, 0x05, 0x09, 0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01, 0x85, 0x02, 0x95, 0x03, 0x75, 0x01, 0x81, 0x02, 0x95, 0x01, 0x75, 0x05, 0x81, 0x01, 0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, 0x02, 0x81, 0x06, 0x95, 0x04, 0x75, 0x08, 0x81, 0x01, 0xc0, 0xc0, 0x05, 0x0d, 0x09, 0x05, 0xa1, 0x01, 0x06, 0x00, 0xff, 0x09, 0x0c, 0x15, 0x00, 0x26, 0xff, 0x00, 0x75, 0x08, 0x95, 0x10, 0x85, 0x3f, 0x81, 0x22, 0xc0, 0x06, 0x00, 0xff, 0x09, 0x0c, 0xa1, 0x01, 0x06, 0x00, 0xff, 0x09, 0x0c, 0x15, 0x00, 0x26, 0xff, 0x00, 0x85, 0x44, 0x75, 0x08, 0x96, 0x6b, 0x05, 0x81, 0x00, 0xc0};

UInt16 abs(SInt16 x){
    if (x < 0)
        return x * -1;
    return x;
}

void VoodooI2CMT2SimulatorDevice::constructReport(VoodooI2CMultitouchEvent multitouch_event, AbsoluteTime timestamp) {
    if (!ready_for_reports)
        return;

    command_gate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooI2CMT2SimulatorDevice::constructReportGated), &multitouch_event, &timestamp);
}

void VoodooI2CMT2SimulatorDevice::constructReportGated(VoodooI2CMultitouchEvent& multitouch_event, AbsoluteTime& timestamp) {
    if (!ready_for_reports)
        return;

    MAGIC_TRACKPAD_INPUT_REPORT input_report;
    input_report.ReportID = 0x02;
    input_report.Unused[0] = 0;
    input_report.Unused[1] = 0;
    input_report.Unused[2] = 0;
    input_report.Unused[3] = 0;
    input_report.Unused[4] = 0;
    
    VoodooI2CDigitiserTransducer* transducer = OSDynamicCast(VoodooI2CDigitiserTransducer, multitouch_event.transducers->getObject(0));
    
    if (!transducer)
        return;
    
    // physical button
    input_report.Button = transducer->physical_button.value();
    
    // touch active
    
    // multitouch report id
    input_report.multitouch_report_id = 0x31; //Magic
    
    // timestamp
    AbsoluteTime relative_timestamp = timestamp;
    SUB_ABSOLUTETIME(&relative_timestamp, &start_timestamp);
    
    UInt64 milli_timestamp;
    
    absolutetime_to_nanoseconds(relative_timestamp, &milli_timestamp);
    
    milli_timestamp /= 1000000;
    
    input_report.timestamp_buffer[0] = (milli_timestamp << 0x3) | 0x4;
    input_report.timestamp_buffer[1] = (milli_timestamp >> 0x5) & 0xFF;
    input_report.timestamp_buffer[2] = (milli_timestamp >> 0xd) & 0xFF;
    
    // finger data
    int first_unknownbit = -1;
    bool input_active = false;
    
    for (int i = 0; i < multitouch_event.contact_count + 1; i++) {
        VoodooI2CDigitiserTransducer* transducer = OSDynamicCast(VoodooI2CDigitiserTransducer, multitouch_event.transducers->getObject(i));
        
        new_touch_state[i] = touch_state[i];
        touch_state[i] = 0;
        
        if (!transducer || !transducer->is_valid)
            continue;

        if (transducer->type == kDigitiserTransducerStylus) {
            continue;
        }
        
        if (!transducer->tip_switch.value()) {
            new_touch_state[i] = 0;
            touch_state[i] = 0;
        } else {
            input_active = true;
        }

        MAGIC_TRACKPAD_INPUT_REPORT_FINGER& finger_data = input_report.FINGERS[i];
        
        SInt16 x_min = 3678;
        SInt16 y_min = 2479;
        
        IOFixed scaled_x = ((transducer->coordinates.x.value() * 1.0f) / engine->interface->logical_max_x) * 7612;
        IOFixed scaled_y = ((transducer->coordinates.y.value() * 1.0f) / engine->interface->logical_max_y) * 5065;
        
        IOFixed scaled_old_x = ((transducer->coordinates.x.last.value * 1.0f) / engine->interface->logical_max_x) * 7612;
        uint8_t scaled_old_x_truncated = scaled_old_x;
        
        new_touch_state[i]++;
        touch_state[i] = new_touch_state[i];
        
        int newunknown = stashed_unknown[i];
        
        if (abs(scaled_x - scaled_old_x_truncated) > 50){
            if (scaled_x <= 23){
                newunknown = 0x44;
            } else if (scaled_x <= 27){
                newunknown = 0x64;
            } else if (scaled_x <= 37){
                newunknown = 0x84;
            } else if (scaled_x <= 2307){
                newunknown = 0x94;
            } else if (scaled_x <= 3059){
                newunknown = 0x90;
            } else if (scaled_x <= 4139){
                newunknown = 0x8c;
            } else if (scaled_x <= 5015){
                newunknown = 0x88;
            } else if (scaled_x <= 7553){
                newunknown = 0x94;
            } else if (scaled_x <= 7600){
                newunknown = 0x84;
            } else if (scaled_x <= 7605){
                newunknown = 0x64;
            } else {
                newunknown = 0x44;
            }
        }

        if(first_unknownbit == -1) {
            first_unknownbit = newunknown;
        }
        newunknown = first_unknownbit - (4 * i);
        
        if (new_touch_state[i] == 1) {
            newunknown = 0x20;
            finger_data.Size = 0x0;
        } else if (new_touch_state[i] == 2) {
            newunknown = 0x70;
            finger_data.Size = 0x8;
        } else if (new_touch_state[i] == 3) {
            finger_data.Size = 0x10;
        } else if (new_touch_state[i] == 4) {
            finger_data.Size = 0x20;
            // finger_data.Size = 0x20 + (5 * touch_state[i]);
            
            // if(finger_data.Size >= 100)
               // finger_data.Size = 100;
        }

        if (!transducer->tip_switch.value()) {
            newunknown = 0xF4;
            finger_data.Size = 0x0;
        }
        
        stashed_unknown[i] = newunknown;
        
        SInt16 adjusted_x = scaled_x - x_min;
        SInt16 adjusted_y = scaled_y - y_min;
        adjusted_y = adjusted_y * -1;
        
        uint16_t rawx = *(uint16_t *)&adjusted_x;
        uint16_t rawy = *(uint16_t *)&adjusted_y;
        
        finger_data.AbsX = rawx & 0xff;
        
        finger_data.AbsXY = 0;
        finger_data.AbsXY |= (rawx >> 8) & 0x0f;
        if ((rawx >> 15) & 0x01)
            finger_data.AbsXY |= 0x10;
        
        finger_data.AbsXY |= (rawy << 5) & 0xe0;
        finger_data.AbsY[0] = (rawy >> 3) & 0xff;
        finger_data.AbsY[1] = (rawy >> 11) & 0x01;
        if ((rawy >> 15) & 0x01)
            finger_data.AbsY[1] |= 0x02;
        
        finger_data.AbsY[1] |= newunknown;
        
        if (transducer->tip_switch.value()) {
            finger_data.Touch_Major = 128;
            finger_data.Touch_Minor = 128;
        } else {
            finger_data.Touch_Major = 0;
            finger_data.Touch_Minor = 0;
        }

        if (transducer->tip_pressure.value() || (i == 0 && input_report.Button))
            finger_data.Pressure = 120;
        else if (!transducer->tip_switch.value())
            finger_data.Pressure = 0;
        else
            finger_data.Pressure = 30;
        
        finger_data.Orientation_Origin = (128 & 0xF0) | ((transducer->secondary_id + 1) & 0xF);
    }

    if (input_active)
        input_report.TouchActive = 0x3;
    else
        input_report.TouchActive = 0x2;
    
    int total_report_len = (9 * multitouch_event.contact_count) + 12;
    IOBufferMemoryDescriptor* buffer_report = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, total_report_len);
    buffer_report->writeBytes(0, &input_report, total_report_len);
    
    handleReport(buffer_report, kIOHIDReportTypeInput);
    
    if (!input_active) {
        buffer_report = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, 12);
        buffer_report->writeBytes(0, &input_report, 12);
        handleReport(buffer_report, kIOHIDReportTypeInput);
    }
}

bool VoodooI2CMT2SimulatorDevice::start(IOService* provider) {
    if (!super::start(provider))
        return false;
    
    clock_get_uptime(&start_timestamp);
    
    engine = OSDynamicCast(VoodooI2CNativeEngine, provider);
    
    if (!engine)
        return false;

    workLoop = this->getWorkLoop();
    if (!workLoop) {
        IOLog("%s Could not get a IOWorkLoop instance\n", getName());
        return false;
    }
    
    workLoop->retain();
    
    command_gate = IOCommandGate::commandGate(this);
    if (!command_gate || (workLoop->addEventSource(command_gate) != kIOReturnSuccess)) {
        IOLog("%s Could not open command gate\n", getName());
        workLoop->release();
        workLoop = NULL;
        
        return false;
    }
    
    workLoop = this->getWorkLoop();
    if (!workLoop) {
        IOLog("%s Could not get a IOWorkLoop instance\n", getName());
        return false;
    }
    
    workLoop->retain();
    
    command_gate = IOCommandGate::commandGate(this);
    if (!command_gate || (workLoop->addEventSource(command_gate) != kIOReturnSuccess)) {
        IOLog("%s Could not open command gate\n", getName());
        workLoop->release();
        workLoop = NULL;
        
        return false;
    }
    
    ready_for_reports = true;
    
    return true;
}

IOReturn VoodooI2CMT2SimulatorDevice::setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options) {
    
    IOLog("MT2 requested setReport with report_id: %d\n", options & 0xFF);
    
    UInt32 report_id = options & 0xFF;
    
    if (report_id == 0x1) {
        char* raw_buffer = (char*)IOMalloc(report->getLength());
        
        report->prepare();
        
        report->readBytes(0, raw_buffer, report->getLength());
        
        report->complete();
        
        new_get_report_buffer = OSData::withCapacity(1);
        
        UInt8 value = raw_buffer[1];
        
        IOLog("Got here with value: 0x%x\n", value);
        
        if (value == 0xDB) {
            unsigned char buffer[] = {0x1, 0xDB, 0x00, 0x49, 0x00};
            new_get_report_buffer->appendBytes(buffer, sizeof(buffer));
        }
        
        if (value == 0xD1) {
            unsigned char buffer[] = {0x1, 0xD1, 0x00, 0x01, 0x00};
            new_get_report_buffer->appendBytes(buffer, sizeof(buffer));
        }
        
        if (value == 0xD3) {
            unsigned char buffer[] = {0x1, 0xD3, 0x00, 0x0C, 0x00};
            new_get_report_buffer->appendBytes(buffer, sizeof(buffer));
        }
        
        if (value == 0xD0) {
            unsigned char buffer[] = {0x1, 0xD0, 0x00, 0x0F, 0x00};
            new_get_report_buffer->appendBytes(buffer, sizeof(buffer));
        }
        
        if (value == 0xA1) {
            unsigned char buffer[] = {0x1, 0xA1, 0x00, 0x06, 0x00};
            new_get_report_buffer->appendBytes(buffer, sizeof(buffer));
        }
        
        if (value == 0xD9) {
            unsigned char buffer[] = {0x1, 0xD9, 0x00, 0x10, 0x00};
            new_get_report_buffer->appendBytes(buffer, sizeof(buffer));
        }
        
        if (value == 0x7F) {
            unsigned char buffer[] = {0x1, 0x7F, 0x00, 0x04, 0x00};
            new_get_report_buffer->appendBytes(buffer, sizeof(buffer));
        }
        
        if (value == 0xC8) {
            unsigned char buffer[] = {0x1, 0xC8, 0x00, 0x01, 0x00};
            new_get_report_buffer->appendBytes(buffer, sizeof(buffer));
        }
        
        IOFree(raw_buffer, report->getLength());
    }
    
    return kIOReturnSuccess;
}

IOReturn VoodooI2CMT2SimulatorDevice::getReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options) {
    IOLog("MT2 requested getReport! report_id: %d\n", options & 0xFF);
    UInt32 report_id = options & 0xFF;
    
    OSData* get_buffer = OSData::withCapacity(1);
    
    if (report_id == 0x0) {
        unsigned char buffer[] = {0x0, 0x01};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0x1) {
        IOLog("We are going to send this: 0x%llx\n", (UInt64)new_get_report_buffer->getBytesNoCopy());
        get_buffer = new_get_report_buffer;
    }
    
    if (report_id == 0xD1) {
        unsigned char buffer[] = {0xD1, 0x81};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0xD3) {
        unsigned char buffer[] = {0xD3, 0x01, 0x16, 0x1E, 0x03, 0x95, 0x00, 0x14, 0x1E, 0x62, 0x05, 0x00, 0x00};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0xD0) {
        unsigned char buffer[] = {0xD0, 0x02, 0x01, 0x00, 0x14, 0x01, 0x00, 0x1E, 0x00, 0x02, 0x14, 0x02, 0x01, 0x0E, 0x02, 0x00};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0xA1) {
        unsigned char buffer[] = {0xA1, 0x00, 0x00, 0x05, 0x00, 0xFC, 0x01};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0xD9) {
        //0xD9, 0xF0, 0x3C, 0x00, 0x00, 0x20, 0x2B, 0x00, 0x00, 0x44, 0xE3, 0x52, 0xFF, 0xBD, 0x1E, 0xE4, 0x26
        unsigned char buffer[] = {0xD9, 0xF0, 0x3C, 0x00, 0x00, 0x20, 0x2B, 0x00, 0x00, 0x44, 0xE3, 0x52, 0xFF, 0xBD, 0x1E, 0xE4, 0x26};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0x7F) {
        unsigned char buffer[] = {0x7F, 0x00, 0x00, 0x00, 0x00};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0xC8) {
        unsigned char buffer[] = {0xC8, 0x08};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0x2) {
        unsigned char buffer[] = {0x02, 0x01};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0xDB) {
        unsigned char buffer[] = {0xDB, 0x01, 0x02, 0x00, 0xD1, 0x81, 0x0D, 0x00, 0xD3, 0x01, 0x16, 0x1E, 0x03, 0x95, 0x00, 0x14, 0x1E, 0x62, 0x05, 0x00, 0x00, 0x10, 0x00, 0xD0, 0x02, 0x01, 0x00, 0x14, 0x01, 0x00, 0x1E, 0x00, 0x02, 0x14, 0x02, 0x01, 0x0E, 0x02, 0x00, 0x07, 0x00, 0xA1, 0x00, 0x00, 0x05, 0x00, 0xFC, 0x01, 0x11, 0x00, 0xD9, 0xF0, 0x3C, 0x00, 0x00, 0x20, 0x2B, 0x00, 0x00, 0x44, 0xE3, 0x52, 0xFF, 0xBD, 0x1E, 0xE4, 0x26, 0x05, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    report->writeBytes(0, get_buffer->getBytesNoCopy(), get_buffer->getLength());
    
    return kIOReturnSuccess;
}

IOReturn VoodooI2CMT2SimulatorDevice::newReportDescriptor(IOMemoryDescriptor** descriptor) const {
    IOBufferMemoryDescriptor* report_descriptor_buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, sizeof(report_descriptor));
    
    if (!report_descriptor_buffer) {
        IOLog("%s Could not allocated buffer for report descriptor\n", getName());
        return kIOReturnNoResources;
    }
    
    report_descriptor_buffer->writeBytes(0, report_descriptor, sizeof(report_descriptor));
    *descriptor = report_descriptor_buffer;
    
    return kIOReturnSuccess;
}

OSString* VoodooI2CMT2SimulatorDevice::newManufacturerString() const {
    return OSString::withCString("Apple Inc.");
}

OSNumber* VoodooI2CMT2SimulatorDevice::newPrimaryUsageNumber() const {
    return OSNumber::withNumber(kHIDUsage_GD_Mouse, 32);
}

OSNumber* VoodooI2CMT2SimulatorDevice::newPrimaryUsagePageNumber() const {
    return OSNumber::withNumber(kHIDPage_GenericDesktop, 32);
}

OSNumber* VoodooI2CMT2SimulatorDevice::newProductIDNumber() const {
    return OSNumber::withNumber(0x265, 32);
}

OSString* VoodooI2CMT2SimulatorDevice::newProductString() const {
    return OSString::withCString("Magic Trackpad 2");
}

OSString* VoodooI2CMT2SimulatorDevice::newSerialNumberString() const {
    return OSString::withCString("Voodoo Magic Trackpad Simulator");
}

OSString* VoodooI2CMT2SimulatorDevice::newTransportString() const {
    return OSString::withCString("USB");
}

OSNumber* VoodooI2CMT2SimulatorDevice::newVendorIDNumber() const {
    return OSNumber::withNumber(0x5ac, 16);
}

OSNumber* VoodooI2CMT2SimulatorDevice::newLocationIDNumber() const {
    return OSNumber::withNumber(0x14400000, 32);
}

OSNumber* VoodooI2CMT2SimulatorDevice::newVersionNumber() const {
    return OSNumber::withNumber(0x804, 32);
}
