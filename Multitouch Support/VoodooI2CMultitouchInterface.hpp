//
//  VoodooI2CMultitouchInterface.hpp
//  VoodooI2C
//
//  Created by Alexandre on 22/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CMultitouchInterface_hpp
#define VoodooI2CMultitouchInterface_hpp

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#include "MultitouchHelpers.hpp"
#include "VoodooI2CDigitiserStylus.hpp"

class VoodooI2CMultitouchEngine;

class VoodooI2CMultitouchInterface : public IOService {
  OSDeclareDefaultStructors(VoodooI2CMultitouchInterface);

 public:
    UInt32 logical_max_x = 0;
    UInt32 logical_max_y = 0;
    UInt32 physical_max_x = 0;
    UInt32 physical_max_y = 0;

    void handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp);
    bool open(IOService* client);
    static SInt8 orderEngines(VoodooI2CMultitouchEngine* a, VoodooI2CMultitouchEngine* b);
    bool start(IOService* provider);
    void stop(IOService* provider);

 protected:
 private:
    OSOrderedSet* engines;
};


#endif /* VoodooI2CMultitouchInterface_hpp */
