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

#ifndef EXPORT
#define EXPORT __attribute__((visibility("default")))
#endif

struct ValueWithTime {
    UInt16 value = 0;
    AbsoluteTime timestamp = 0;
};

/* Tracks a <UInt16> value with the current and last value being accessible with their respective timestamps */

class TimeTrackedValue {
public:
    ValueWithTime current;
    ValueWithTime last;
    
    operator bool() {
        return (bool)current.value;
    }
    
    /* Updates a timetracked value
     * @value The new value to be set
     * @timestamp The timestamp of the new value
     */
    
    void update(UInt16 value, AbsoluteTime timestamp) {
        last = current;
        current.value = value;
        current.timestamp = timestamp;
    }
    
    /* Returns the current value */
    
    UInt16 value() {
        return current.value;
    }
};

typedef TimeTrackedValue DigitiserTransducerButtonState;

typedef enum {
    kDigitiserTransducerFinger,
    kDigitiserTransducerStylus,
    kDigitiserTransducerPuck
} DigitiserTransducerType;

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

/* Represents a transducer of a physical digitiser device */

class EXPORT VoodooI2CDigitiserTransducer : public OSObject {
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
    
    TimeTrackedValue touch_major;
    TimeTrackedValue touch_minor;
    
    SInt32 logical_max_x;
    SInt32 logical_max_y;
    SInt32 logical_max_z;
    SInt32 pressure_physical_max;

    UInt16 id;
    UInt16 secondary_id;
    
    bool in_range = false;
    bool is_valid = false;
    DigitiserTransducerType type;
    
    IOHIDElement*  collection;
    
    UInt32 event_mask;
    AbsoluteTime timestamp;
    
    bool serialize(OSSerialize* serializer);
    
    /* Instantiates a new transducer
     * @transducer_type The type of transducer to be created
     * @digitizer_collection The HID element associated to this transducer. Set to *NULL* if not in an HID context
     *
     * @return A pointer to an instance of <VoodooI2CDigitiserTransducer>
     */
    
    static VoodooI2CDigitiserTransducer* transducer(DigitiserTransducerType transducer_type, IOHIDElement* digitizer_collection);
protected:
private:
};


#endif /* VoodooI2CDigitiserTransducer_hpp */
