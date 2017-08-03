//
//  helpers.hpp
//  VoodooI2C
//
//  Created by Alexandre on 31/07/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef helpers_hpp
#define helpers_hpp

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>

const char* getMatchedName(IOService* provider);

enum VoodooI2CPowerState {
    kVoodooI2CPowerStateOff = 0,
    kVoodooI2CPowerStateOn = 1
};

#endif /* helpers_hpp */
