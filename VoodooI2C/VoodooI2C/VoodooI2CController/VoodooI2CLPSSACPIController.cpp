//
//  VoodooI2CLPSSACPIController.hpp
//  VoodooI2C
//
//  Created by Avery Black on 11/23/2025.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#include "VoodooI2CLPSSACPIController.hpp"

#define LPSS_PRIVATE_CLOCK_GATING              0x800

#define super VoodooI2CACPIController
OSDefineMetaClassAndStructors(VoodooI2CLPSSACPIController, VoodooI2CACPIController);

void VoodooI2CLPSSACPIController::setClockGating(VoodooI2CState enabled) {
    writeRegister(enabled, LPSS_PRIVATE_CLOCK_GATING);
}

IOReturn VoodooI2CLPSSACPIController::setPowerState(unsigned long whichState, IOService * whatDevice) {
    if (whatDevice != this)
        return kIOPMAckImplied;

    if (whichState == 0) {  // index of kIOPMPowerOff state in VoodooI2CIOPMPowerStates
        setClockGating(kVoodooI2CStateOff);
    } else {
        setClockGating(kVoodooI2CStateOn);
    }

    return super::setPowerState(whichState, whatDevice);
}

bool VoodooI2CLPSSACPIController::start(IOService* provider) {
    if (!super::start(provider)) {
        return false;
    }

    setClockGating(kVoodooI2CStateOn);
    return true;
}

void VoodooI2CLPSSACPIController::stop(IOService* provider) {
    setClockGating(kVoodooI2CStateOff);
    super::stop(provider);
}
