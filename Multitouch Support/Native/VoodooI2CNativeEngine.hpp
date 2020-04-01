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

#include "../VoodooI2CMultitouchInterface.hpp"
#include "../VoodooI2CMultitouchEngine.hpp"


#include "../../Dependencies/VoodooInput/VoodooInput/VoodooInputMultitouch/VoodooInputTransducer.h"
#include "../../Dependencies/VoodooInput/VoodooInput/VoodooInputMultitouch/VoodooInputMessages.h"
#include "../../Dependencies/helpers.hpp"

class EXPORT VoodooI2CNativeEngine : public VoodooI2CMultitouchEngine {
    OSDeclareDefaultStructors(VoodooI2CNativeEngine);
    
    VoodooInputEvent message;
    VoodooI2CMultitouchInterface* parentProvider;
    IOService* voodooInputInstance;
    IOService* voodooI2CHIDInstance;
 public:
    bool start(IOService* provider) override;
    void stop(IOService* provider) override;
    
    bool handleOpen(IOService *forClient, IOOptionBits options, void *arg) override;
    void handleClose(IOService *forClient, IOOptionBits options) override;
    
    MultitouchReturn handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp);
};


#endif /* VoodooI2CNativeEngine_hpp */
