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

#define kIOPMPowerOff                       0
#define kVoodooI2CIOPMNumberPowerStates     2

#define BIT(nr) (1UL << (nr))

const char* getMatchedName(IOService* provider);

enum VoodooI2CState {
    kVoodooI2CStateOff = 0,
    kVoodooI2CStateOn = 1
};

static IOPMPowerState VoodooI2CIOPMPowerStates[kVoodooI2CIOPMNumberPowerStates] = {
    {1, kIOPMPowerOff, kIOPMPowerOff, kIOPMPowerOff, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, kIOPMPowerOn, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};

#endif /* helpers_hpp */
