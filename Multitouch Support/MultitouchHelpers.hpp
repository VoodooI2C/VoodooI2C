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

typedef struct {
    UInt8 contact_count;
    OSArray* transducers;
} VoodooI2CMultitouchEvent;

typedef UInt32 MultitouchReturn;

#define MultitouchReturnContinue 0x0
#define MultitouchReturnBreak 0x1

#ifndef EXPORT
#define EXPORT __attribute__((visibility("default")))
#endif

#endif /* MultitouchHelpers_hpp */
