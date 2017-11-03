//
//  VoodooI2CDigitiserTransducer.hpp
//  VoodooI2C
//
//  Created by Alexandre on 13/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CDigitiserTransducer_hpp
#define VoodooI2CDigitiserTransducer_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <libkern/c++/OSObject.h>
#include <libkern/c++/OSDictionary.h>
#include <IOKit/hid/IOHIDElement.h>

struct ValueWithTime {
    UInt16 value = 0;
    AbsoluteTime timestamp = 0;
};

class TimeTrackedValue {
public:
    ValueWithTime current;
    ValueWithTime last;

    operator bool() {
        return (bool)current.value;
    }

    void update(UInt16 value, AbsoluteTime timestamp) {
        last = current;
        current.value = value;
        current.timestamp = timestamp;
    }
    
    UInt16 value() {
        return current.value;
    }
};

typedef TimeTrackedValue DigitiserTransducerButtonState;

typedef enum {
    kDigitiserTransducerFinger,
    kDigitiserTransducerStylus,
    kDigitiserTransducerPuck
} DigitiserTransducuerType;

typedef struct {
    TimeTrackedValue x;
    TimeTrackedValue y;
    TimeTrackedValue z;
} DigitiserTransducerCoordinates;

typedef struct {
    TimeTrackedValue width;
    TimeTrackedValue height;
} DigitiserTransducerDimensions;

typedef struct {
    TimeTrackedValue azimuth;
    TimeTrackedValue altitude;
    TimeTrackedValue twist;
} DigitiserTransducerAziAltiOrentation;

typedef struct {
    TimeTrackedValue x_tilt;
    TimeTrackedValue y_tilt;
} DigitiserTransducerTiltOrientation;

class VoodooI2CDigitiserTransducer : public OSObject {
  OSDeclareDefaultStructors(VoodooI2CDigitiserTransducer);

 public:
    DigitiserTransducerButtonState physical_button;

    DigitiserTransducerCoordinates coordinates;
    DigitiserTransducerCoordinates last_coordinates;

    DigitiserTransducerDimensions dimensions;
    DigitiserTransducerDimensions last_dimensions;

    DigitiserTransducerAziAltiOrentation azi_alti_orientation;
    DigitiserTransducerTiltOrientation tilt_orientation;
    
    DigitiserTransducerButtonState tip_switch;
    TimeTrackedValue tip_pressure;
    
    SInt16 logical_max_x;
    SInt16 logical_max_y;
    SInt16 logical_max_z;
    SInt16 pressure_physical_max;

    UInt16 id;
    UInt16 secondary_id;

    bool in_range = false;
    bool is_valid = false;
    DigitiserTransducuerType type;

    IOHIDElement*  collection;
    OSArray*       elements;
    
    UInt32 event_mask;
    AbsoluteTime timestamp;

    void           free();
    bool serialize(OSSerialize* serializer);
    static VoodooI2CDigitiserTransducer* transducer(DigitiserTransducuerType transducer_type, IOHIDElement* digitizer_collection);
 protected:
 private:
};


#endif /* VoodooI2CDigitiserTransducer_hpp */
