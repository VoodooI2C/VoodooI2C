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

class VoodooI2CPCIController : public VoodooI2CController {
    OSDeclareDefaultStructors(VoodooI2CPCIController);

 public:
 private:
    void configurePCI();
    IOReturn getACPIDevice();
    IOReturn setPowerState(unsigned long whichState, IOService * whatDevice);
    inline void skylakeLPSSResetHack();
    bool start(IOService* provider);
    void stop(IOService* provider);
};

#endif /* VoodooI2CPCIController_hpp */
