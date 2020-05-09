//
//  VoodooI2CDigitiserStylus.hpp
//  VoodooI2C
//
//  Created by Alexandre on 13/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CDigitiserStylus_hpp
#define VoodooI2CDigitiserStylus_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#include "VoodooI2CDigitiserTransducer.hpp"


/* Represents a stylus-type transducer */

class EXPORT VoodooI2CDigitiserStylus : public VoodooI2CDigitiserTransducer {
  OSDeclareDefaultStructors(VoodooI2CDigitiserStylus);

 public:
    UInt16 battery_strength;
    DigitiserTransducerButtonState barrel_switch;
    TimeTrackedValue barrel_pressure;
    DigitiserTransducerButtonState eraser;
    bool invert;
    
    static VoodooI2CDigitiserStylus* stylus(DigitiserTransducerType transducer_type, IOHIDElement* digitizer_collection);
};

#endif /* VoodooI2CDigitiserStylus_hpp */
