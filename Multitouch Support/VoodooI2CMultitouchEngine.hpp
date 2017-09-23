//
//  VoodooI2CMultitouchEngine.hpp
//  VoodooI2C
//
//  Created by Alexandre on 22/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CMultitouchEngine_hpp
#define VoodooI2CMultitouchEngine_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#include "MultitouchHelpers.hpp"

class VoodooI2CMultitouchInterface;

class VoodooI2CMultitouchEngine : public IOService {
  OSDeclareDefaultStructors(VoodooI2CMultitouchEngine);

 public:
    virtual UInt8 getScore();
    virtual MultitouchReturn handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp);
    virtual bool start(IOService* provider);
    virtual void stop(IOService* provider);

 protected:
    VoodooI2CMultitouchInterface* interface;
 private:
};


#endif /* VoodooI2CMultitouchEngine_hpp */
