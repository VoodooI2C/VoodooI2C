//
//  VoodooI2CACPIController.hpp
//  VoodooI2C
//
//  Created by Alexandre on 02/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CACPIController_hpp
#define VoodooI2CACPIController_hpp

#include "./VoodooI2CController.hpp"

/* Implements an ACPI Intel LPSS Designware I2C Controller
 *
 * The members of this class are responsible for low-level interfacing with the physical ACPI hardware.
 */

class EXPORT VoodooI2CACPIController : public VoodooI2CController {
    OSDeclareDefaultStructors(VoodooI2CACPIController);

 private:
    /* @inherit */

    IOReturn setPowerState(unsigned long whichState, IOService * whatDevice);

    /* Instructs the controller to enter a specific power state by evaluating the ACPI power state methods
     * @enabled The power state the device is expected to enter represented by either <kVoodooI2CStateOn> or
     *  <kVoodooI2CStateOff>
     *
     * @return *kIOReturnSuccess* on successful state change, *kIOReturnNoPower* otherwise
     */

    IOReturn setACPIPowerState(VoodooI2CState enabled);

    /* @inherit */

    bool start(IOService* provider);

    /* @inherit */

    void stop(IOService* provider);
};

#endif /* VoodooI2CACPIController_hpp */
