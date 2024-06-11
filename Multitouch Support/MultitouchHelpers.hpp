//
//  MultitouchHelpers.hpp
//  VoodooI2C
//
//  Created by Alexandre on 22/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef MultitouchHelpers_hpp
#define MultitouchHelpers_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOService.h>

struct VoodooI2CMultitouchEvent {
    UInt8 contact_count;
    OSArray* transducers;
};

struct VoodooI2CTrackpointEvent {
    UInt8 dx, dy;
    UInt8 buttons;
};

typedef UInt32 MultitouchReturn;

#define MultitouchReturnContinue 0x0
#define MultitouchReturnBreak 0x1

#ifndef EXPORT
#define EXPORT __attribute__((visibility("default")))
#endif

#endif /* MultitouchHelpers_hpp */
