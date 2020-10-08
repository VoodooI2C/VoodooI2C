//
//  VoodooI2CPCILakeController.hpp
//  VoodooI2C
//
//  Created by Zhen on 10/3/20.
//  Copyright © 2020 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CPCILakeController_hpp
#define VoodooI2CPCILakeController_hpp

#include "VoodooI2CPCIController.hpp"

/* Override power management for Comet Lake and newer platforms
 *
 * The members of this class are responsible for low-level interfacing with the physical PCI hardware.
 */

class EXPORT VoodooI2CPCILakeController : public VoodooI2CPCIController {
    OSDeclareDefaultStructors(VoodooI2CPCILakeController);

 private:
    void configurePCI() APPLE_KEXT_OVERRIDE;
};
#endif /* VoodooI2CPCILakeController_hpp */
