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


#include "VoodooInputMultitouch/VoodooInputTransducer.h"
#include "VoodooInputMultitouch/VoodooInputMessages.h"

class EXPORT VoodooI2CNativeEngine : public VoodooI2CMultitouchEngine {
    OSDeclareDefaultStructors(VoodooI2CNativeEngine);
    
    VoodooInputEvent message;
    VoodooI2CMultitouchInterface* parentProvider;
    IOService* voodooInputInstance;

    bool lastIsForceClickEnabled = true;
    AbsoluteTime lastForceClickPropertyUpdateTime;

    bool isForceClickEnabled();
 public:
    bool start(IOService* provider) override;
    void stop(IOService* provider) override;
    
    bool handleOpen(IOService *forClient, IOOptionBits options, void *arg) override;
    bool handleIsOpen(const IOService *forClient) const override;
    void handleClose(IOService *forClient, IOOptionBits options) override;
    void onPropertyChange() override;
    
    MultitouchReturn handleInterruptReport(VoodooI2CMultitouchEvent event, AbsoluteTime timestamp);
 private:
    int stylus_check = 0;
};


#endif /* VoodooI2CNativeEngine_hpp */
