//
//  VoodooI2CLPSSACPIController.hpp
//  VoodooI2C
//
//  Created by Avery Black on 11/23/2025.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CLPSSACPIController_hpp
#define VoodooI2CLPSSACPIController_hpp

#include "VoodooI2CACPIController.hpp"

/* Implements a legacy Intel LPSS wrapper for ACPI Synopsys DesignWare I2C Controllers
 *
 * Implements Intel LPSS logic for clock gating
 */

class EXPORT VoodooI2CLPSSACPIController : public VoodooI2CACPIController {
    OSDeclareDefaultStructors(VoodooI2CLPSSACPIController);

 private:
    /* @inherit */

    IOReturn setPowerState(unsigned long whichState, IOService * whatDevice) override;

    /* @inherit */

    bool start(IOService* provider) override;

    /* @inherit */

    void stop(IOService* provider) override;

    /*
     * Enable/Disable clock gating for I2C controller
     * @enabled The power state the device is expected to enter represented by either
     * <kVoodooI2CStateOn> or <kVoodooI2CStateOff>
     */
    void setClockGating(VoodooI2CState enabled);
};

#endif /* VoodooI2CLPSSACPIController_hpp */
