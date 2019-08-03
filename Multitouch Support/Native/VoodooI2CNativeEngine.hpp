//
//  VoodooI2CNativeEngine.hpp
//  VoodooI2C
//
//  Created by Alexandre on 10/02/2018.
//  Copyright © 2018 Alexandre Daoud and Kishor Prins. All rights reserved.
//

#ifndef VoodooI2CNativeEngine_hpp
#define VoodooI2CNativeEngine_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#include "../VoodooI2CMultitouchEngine.hpp"
#include "VoodooI2CMT2SimulatorDevice.hpp"
#include "VoodooI2CMT2ActuatorDevice.hpp"

class EXPORT VoodooI2CNativeEngine : public VoodooI2CMultitouchEngine {
  OSDeclareDefaultStructors(VoodooI2CNativeEngine);

 public:
    bool init(OSDictionary* properties) override;
    void free() override;

    bool start(IOService* provider) override;
    void stop(IOService* provider) override;
    
    MultitouchReturn handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp);
 private:
    VoodooI2CMT2SimulatorDevice* simulator;
    VoodooI2CMT2ActuatorDevice* actuator;
};


#endif /* VoodooI2CNativeEngine_hpp */
