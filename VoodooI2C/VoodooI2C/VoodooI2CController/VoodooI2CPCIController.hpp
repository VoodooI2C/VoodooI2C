//
//  VoodooI2CPCIController.hpp
//  VoodooI2C
//
//  Created by Alexandre on 02/08/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CPCIController_hpp
#define VoodooI2CPCIController_hpp

#define LPSS_PRIV                   (0x200)
#define LPSS_PRIV_RESETS            (0x04)
#define LPSS_PRIV_RESETS_FUNC       (2<<1)
#define LPSS_PRIV_RESETS_IDMA       (0x3)

#include "VoodooI2CController.hpp"

/* Implements a PCI Intel LPSS Synopsys DesignWare I2C Controller
 *
 * The members of this class are responsible for low-level interfacing with the physical PCI hardware.
 */

class EXPORT VoodooI2CPCIController : public VoodooI2CController {
    OSDeclareDefaultStructors(VoodooI2CPCIController);

 protected:
    /* Configures the PCI provider
     *
     * This function disables PCI power management (which the controller does not support) by
     * enforcing PCI power state D0. It then enables the PCI bus master and the memory banks.
     */

    virtual void configurePCI();

 private:
    /* Finds the ACPI device associated to the PCI provider
     *
     * Despite a controller being PCI enumerated, some PCs will sill provide bus configuration values (used in
     * <VoodooI2CControllerDriver::getBusConfig>) in the ACPI tables in an ACPI device associated to the controller.
     * This function traverses the IORegistry plane to seek out such an ACPI device.
     *
     * @return *kIOReturnSuccess* on successful search, *kIOReturnError* otherwise
     */

    IOReturn getACPIDevice();

    /* @inherit */

    IOReturn setPowerState(unsigned long whichState, IOService * whatDevice);

    /* Skylake LPSS Reset Hack
     *
     * Brings the controller out from reset. We do this here instead of in the driver in order to avoid having to check
     * what provider type (ACPI vs PCI) the controller has.
     */

    inline void skylakeLPSSResetHack();

    /* @inherit */

    bool start(IOService* provider);

    /* @inherit */

    void stop(IOService* provider);
};

#endif /* VoodooI2CPCIController_hpp */
