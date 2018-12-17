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

void VoodooI2CMT2SimulatorDevice::constructReport(VoodooI2CMultitouchEvent multitouch_event, AbsoluteTime timestamp) {
    if (!ready_for_reports)
        return;

    command_gate->runAction(OSMemberFunctionCast(IOCommandGate::Action, this, &VoodooI2CMT2SimulatorDevice::constructReportGated), &multitouch_event, &timestamp);
}

void VoodooI2CMT2SimulatorDevice::constructReportGated(VoodooI2CMultitouchEvent& multitouch_event, AbsoluteTime& timestamp) {
    if (!ready_for_reports)
        return;

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
    pointing_wrapper->updateRelativeMouse(0,0,transducer->physical_button.value() & 0x1);
    
    /*
    if (multitouch_device_preferences) {
        OSBoolean* tap_preference = OSDynamicCast(OSBoolean, multitouch_device_preferences->getObject("Clicking"));
        multitouch_device_preferences->setObject("Clicking", OSBoolean::withBoolean(tap_preference->getValue() & !transducer->physical_button.value()));
    }
     */

    if (transducer->physical_button.value())
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
        
        IOFixed scaled_x = (((transducer->coordinates.x.value()) * 1.0f) / engine->interface->logical_max_x) * 7612;
        IOFixed scaled_y = (((transducer->coordinates.y.value()) * 1.0f) / engine->interface->logical_max_y) * 5065;
        
        IOFixed scaled_old_x = (((transducer->coordinates.x.last.value)* 1.0f) / engine->interface->logical_max_x) * 7612;
        
        IOFixed max_x_tp = (engine->interface->logical_max_x * (factor_ref / factor_x));
        IOFixed max_y_tp = (engine->interface->logical_max_y * (factor_ref / factor_y));
        
        if (max_x_tp < 7612 && max_y_tp < 5065){
            scaled_x = (transducer->coordinates.x.value() * 1.0f * (factor_ref / factor_x)) + (7612 - max_x_tp);
            scaled_y = (transducer->coordinates.y.value() * 1.0f * (factor_ref / factor_y)) + (5065 - max_y_tp);
            scaled_old_x = (transducer->coordinates.x.last.value * 1.0f) + (7612 - max_x_tp);
        }
        
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
        
        if (new_touch_state[i] > 4) {
            finger_data.Size = 0x30;
            finger_data.Pressure = 30;
            finger_data.Touch_Minor = 128;
            finger_data.Touch_Major = 128;
        } else if (new_touch_state[i] == 1) {
            newunknown = 0x20;
            finger_data.Size = 0x0;
            finger_data.Pressure = 0x0;
            finger_data.Touch_Minor = 0x0;
            finger_data.Touch_Major = 0x0;
        } else if (new_touch_state[i] == 2) {
            newunknown = 0x70;
            finger_data.Size = 0x8;
            finger_data.Pressure = 10;
            finger_data.Touch_Minor = 32;
            finger_data.Touch_Major = 32;
        } else if (new_touch_state[i] == 3) {
            finger_data.Size = 0x10;
            finger_data.Pressure = 20;
            finger_data.Touch_Minor = 64;
            finger_data.Touch_Major = 64;
        } else if (new_touch_state[i] == 4) {
            finger_data.Size = 0x20;
            finger_data.Pressure = 30;
            finger_data.Touch_Minor = 96;
            finger_data.Touch_Major = 96;
        }
        
        
        if (transducer->tip_pressure.value() || (i == 0 && input_report.Button)) {
            finger_data.Pressure = 160;
            input_report.Button = 0x80;
        }
        

        if (!transducer->tip_switch.value()) {
            newunknown = 0xF4;
            finger_data.Size = 0x10;
            finger_data.Pressure = 0x10;
            finger_data.Touch_Minor = 64;
            finger_data.Touch_Major = 64;
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
    buffer_report->release();
    buffer_report = NULL;

    if (!input_active) {
        input_report.FINGERS[0].Size = 0x0;
        input_report.FINGERS[0].Pressure = 0x0;
        input_report.FINGERS[0].Touch_Major = 0x0;
        input_report.FINGERS[0].Touch_Minor = 0x0;

        milli_timestamp += 10;
        
        input_report.timestamp_buffer[0] = (milli_timestamp << 0x3) | 0x4;
        input_report.timestamp_buffer[1] = (milli_timestamp >> 0x5) & 0xFF;
        input_report.timestamp_buffer[2] = (milli_timestamp >> 0xd) & 0xFF;

        buffer_report = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, 12);
        buffer_report->writeBytes(0, &input_report, 12);
        handleReport(buffer_report, kIOHIDReportTypeInput);
        buffer_report->release();
    }
    
    input_report = {};
}

bool VoodooI2CMT2SimulatorDevice::getMultitouchPreferences(void* target, void* ref_con, IOService* multitouch_device, IONotifier* notifier) {
    VoodooI2CMT2SimulatorDevice* simulator = (VoodooI2CMT2SimulatorDevice*)target;
    
    IOLog("VoodooI2C: Got multitouch matched\n");
    
    simulator->multitouch_device_preferences = OSDynamicCast(OSDictionary, simulator->getProperty("MultitouchPreferences", gIOServicePlane, kIORegistryIterateRecursively));

        if (simulator->multitouch_device_preferences) {
            IOLog("VoodooI2C: Got multitouch preferences\n");
            
            simulator->multitouch_device_notifier->disable();
            simulator->multitouch_device_notifier->remove();
        }

    return true;
}

bool VoodooI2CMT2SimulatorDevice::start(IOService* provider) {
    if (!super::start(provider))
        return false;
    
    clock_get_uptime(&start_timestamp);
    
    engine = OSDynamicCast(VoodooI2CNativeEngine, provider);
    
    if (!engine)
        return false;

    work_loop = this->getWorkLoop();
    if (!work_loop) {
        IOLog("%s Could not get a IOWorkLoop instance\n", getName());
        releaseResources();
        return false;
    }
    
    work_loop->retain();
    
    command_gate = IOCommandGate::commandGate(this);
    if (!command_gate || (work_loop->addEventSource(command_gate) != kIOReturnSuccess)) {
        IOLog("%s Could not open command gate\n", getName());
        releaseResources();
        return false;
    }

    PMinit();
    engine->parent->joinPMtree(this);
    registerPowerDriver(this, VoodooI2CIOPMPowerStates, kVoodooI2CIOPMNumberPowerStates);

    factor_x = engine->interface->logical_max_x / engine->interface->physical_max_x;
    factor_y = engine->interface->logical_max_y / engine->interface->physical_max_y;
    
    factor_ref = 4.75;

    if (!factor_x)
        factor_x = 1;

    if (!factor_y)
        factor_y = 1;
    

    pointing_wrapper = new VoodooI2CMT2PointingWrapper;
    if (pointing_wrapper->init()){
        pointing_wrapper->attach(this);
        pointing_wrapper->start(this);
    } else {
        pointing_wrapper->release();
        pointing_wrapper = NULL;
        return false;
    }
    
    multitouch_device_notifier = addMatchingNotification(gIOFirstPublishNotification, IOService::serviceMatching("AppleMultitouchDevice"), VoodooI2CMT2SimulatorDevice::getMultitouchPreferences, this, NULL, 0);
    
    ready_for_reports = true;
    
    return true;
}

void VoodooI2CMT2SimulatorDevice::stop(IOService* provider) {
    if (!pointing_wrapper){
        pointing_wrapper->terminate(kIOServiceRequired | kIOServiceSynchronous);
        pointing_wrapper->release();
        pointing_wrapper = NULL;
    }
    releaseResources();
    
    PMstop();
    
    super::stop(provider);
}

IOReturn VoodooI2CMT2SimulatorDevice::setPowerState(unsigned long whichState, IOService* whatDevice) {
    if (whatDevice != this)
        return kIOReturnInvalid;
    if (whichState == 0){
        // ready_for_reports = false;
    } else {
        // ready_for_reports = true;
    }
    return kIOPMAckImplied;
}

void VoodooI2CMT2SimulatorDevice::releaseResources() {
    if (command_gate) {
        work_loop->removeEventSource(command_gate);
        command_gate->release();
        command_gate = NULL;
    }
    
    if (work_loop)
        OSSafeReleaseNULL(work_loop);
}

IOReturn VoodooI2CMT2SimulatorDevice::setReport(IOMemoryDescriptor* report, IOHIDReportType reportType, IOOptionBits options) {
    
    UInt32 report_id = options & 0xFF;
    
    if (report_id == 0x1) {
        char* raw_buffer = (char*)IOMalloc(report->getLength());
        
        report->prepare();
        
        report->readBytes(0, raw_buffer, report->getLength());
        
        report->complete();
        
        new_get_report_buffer = OSData::withCapacity(1);
        
        UInt8 value = raw_buffer[1];
        
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
    UInt32 report_id = options & 0xFF;
    
    OSData* get_buffer = OSData::withCapacity(1);
    
    if (report_id == 0x0) {
        unsigned char buffer[] = {0x0, 0x01};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0x1) {
        get_buffer = new_get_report_buffer;
    }
    
    if (report_id == 0xD1) {
        unsigned char buffer[] = {0xD1, 0x81};
        //Family ID = 0x81
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0xD3) {
        unsigned char buffer[] = {0xD3, 0x01, 0x16, 0x1E, 0x03, 0x95, 0x00, 0x14, 0x1E, 0x62, 0x05, 0x00, 0x00};
        //Sensor Rows = 0x16
        //Sensor Columns = 0x1e
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0xD0) {
        unsigned char buffer[] = {0xD0, 0x02, 0x01, 0x00, 0x14, 0x01, 0x00, 0x1E, 0x00, 0x02, 0x14, 0x02, 0x01, 0x0E, 0x02, 0x00}; //Sensor Region Description
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0xA1) {
        unsigned char buffer[] = {0xA1, 0x00, 0x00, 0x05, 0x00, 0xFC, 0x01}; //Sensor Region Param
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    if (report_id == 0xD9) {
        unsigned char buffer[] = {0xD9, 0xF0, 0x3C, 0x00, 0x00, 0x20, 0x2B, 0x00, 0x00, 0x44, 0xE3, 0x52, 0xFF, 0xBD, 0x1E, 0xE4, 0x26}; //Sensor Surface Description
        
        //Sensor Surface Width = 0x3cf0 (0xf0, 0x3c) = 15.600 cm
        //Sensor Surface Height = 0x2b20 (0x20, 0x2b) = 11.040 cm
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
        unsigned char buffer[] = {0xDB, 0x01, 0x02, 0x00,
            /* Start 0xD1 */ 0xD1, 0x81, /* End 0xD1 */
            0x0D, 0x00,
            /* Start 0xD3 */ 0xD3, 0x01, 0x16, 0x1E, 0x03, 0x95, 0x00, 0x14, 0x1E, 0x62, 0x05, 0x00, 0x00, /* End 0xD3 */
            0x10, 0x00,
            /* Start 0xD0 */ 0xD0, 0x02, 0x01, 0x00, 0x14, 0x01, 0x00, 0x1E, 0x00, 0x02, 0x14, 0x02, 0x01, 0x0E, 0x02, 0x00, /* End 0xD0 */
            0x07, 0x00,
            /* Start 0xA1 */ 0xA1, 0x00, 0x00, 0x05, 0x00, 0xFC, 0x01, /* End 0xA1 */
            0x11, 0x00,
            /* Start 0xD9 */ 0xD9, 0xF0, 0x3C, 0x00, 0x00, 0x20, 0x2B, 0x00, 0x00, 0x44, 0xE3, 0x52, 0xFF, 0xBD, 0x1E, 0xE4, 0x26, /* End 0xD9 */
            0x05, 0x00,
            /* Start 0x7F */ 0x7F, 0x00, 0x00, 0x00, 0x00 /*End 0x7F */};
        get_buffer->appendBytes(buffer, sizeof(buffer));
    }
    
    report->writeBytes(0, get_buffer->getBytesNoCopy(), get_buffer->getLength());
    
    return kIOReturnSuccess;
}

IOReturn VoodooI2CMT2SimulatorDevice::newReportDescriptor(IOMemoryDescriptor** descriptor) const {
    IOBufferMemoryDescriptor* report_descriptor_buffer = IOBufferMemoryDescriptor::inTaskWithOptions(kernel_task, 0, sizeof(report_descriptor));
    
    if (!report_descriptor_buffer) {
        IOLog("%s Could not allocate buffer for report descriptor\n", getName());
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
    return OSNumber::withNumber(0x290, 32);
}

OSString* VoodooI2CMT2SimulatorDevice::newProductString() const {
    return OSString::withCString("Magic Trackpad 2");
}

OSString* VoodooI2CMT2SimulatorDevice::newSerialNumberString() const {
    return OSString::withCString("VoodooI2C Magic Trackpad 2 Simulator");
}

OSString* VoodooI2CMT2SimulatorDevice::newTransportString() const {
    return OSString::withCString("I2C");
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
